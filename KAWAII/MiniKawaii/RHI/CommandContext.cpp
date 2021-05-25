#include "../pch.h"
#include "CommandContext.h"
#include "RenderDevice.h"
#include "CommandListManager.h"

namespace RHI
{
	CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE type)
	{
		auto& availableContexts = m_AvailableContexts[type];
		CommandContext* context = nullptr;

		if (availableContexts.empty())
		{
			context = new CommandContext(type);
			m_ContextPool[type].emplace_back(context);
			context->Initialize();
		}
		else
		{
			context = availableContexts.front();
			availableContexts.pop();
			context->Reset();
		}

		assert(context != nullptr);

		assert(context->m_type == type);

		return context;
	}

	void ContextManager::FreeContext(CommandContext* usedContext)
	{
		assert(usedContext != nullptr);
		m_AvailableContexts[usedContext->m_type].push(usedContext);
	}

	// ------------------- Command Context ---------------------
	CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
		: m_type(type),
		m_CommandList(nullptr),
		m_CurrentAllocator(nullptr),
		m_numBarriersToFlush(0),
		m_DynamicGPUDescriptorAllocator(RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 128, "DynamicDescriptorMgr"),
		m_DynamicResourceHeap(RenderDevice::GetSingleton().GetDynamicResourceAllocator(), DYNAMIC_RESOURCE_PAGE_SIZE)
	{

	}

	CommandContext::~CommandContext()
	{
		m_DynamicGPUDescriptorAllocator.ReleaseAllocations();
		m_DynamicResourceHeap.ReleaseAllocatedPages();
	}

	void CommandContext::Initialize()
	{
		CommandListManager::GetSingleton().CreateNewCommandList(m_type, m_CommandList.GetAddressOf(), &m_CurrentAllocator);
	}

	void CommandContext::Reset()
	{
		assert(m_CommandList != nullptr && m_CurrentAllocator == nullptr);

		m_CurrentAllocator = CommandListManager::GetSingleton().GetQueue(m_type).RequestAllocator();
		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO
		
	}

	CommandContext& CommandContext::Begin(const std::wstring ID /*= L""*/)
	{
		CommandContext* NewContext = ContextManager::GetSingleton().AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		NewContext->SetID(ID);

		return *NewContext;
	}

	uint64_t CommandContext::Flush(bool waitForCompletion /*= false*/)
	{
		FlushResourceBarriers();

		assert(m_CurrentAllocator != nullptr);

		uint64_t fenceValue = CommandListManager::GetSingleton().GetQueue(m_type).ExecuteCommandList(m_CommandList.Get());

		if (waitForCompletion)
			CommandListManager::GetSingleton().WaitForFence(fenceValue, m_type);
		
		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO: After CommandList Reset, reset the rendering state

		return fenceValue;
	}

	uint64_t CommandContext::Finish(bool waitForCompletion /*= false*/, bool releaseDynamic /*= false*/)
	{
		assert(m_type == D3D12_COMMAND_LIST_TYPE_DIRECT || m_type == D3D12_COMMAND_LIST_TYPE_COMPUTE);

		FlushResourceBarriers();

		assert(m_CurrentAllocator != nullptr);

		CommandQueue& commandQueue = CommandListManager::GetSingleton().GetQueue(m_type);

		// Clean Release Queue
		RenderDevice::GetSingleton().PurgeReleaseQueue(false);

		// Release dynamic resources at the end of each frame
		if (releaseDynamic)
		{
			m_DynamicGPUDescriptorAllocator.ReleaseAllocations();
			m_DynamicResourceHeap.ReleaseAllocatedPages();
		}

		uint64_t fenceValue = commandQueue.ExecuteCommandList(m_CommandList.Get());
		commandQueue.DiscardAllocator(fenceValue, m_CurrentAllocator);
		m_CurrentAllocator = nullptr;

		if (waitForCompletion)
			CommandListManager::GetSingleton().WaitForFence(fenceValue, m_type);

		ContextManager::GetSingleton().FreeContext(this);

		return fenceValue;
	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* data, size_t numBytes, size_t destOffset /*= 0*/)
	{
		CommandContext& initContext = CommandContext::Begin();

		// Copy to UploadBuffer, the UploadBuffer here will be automatically released, and SafeRelease will be called in the destructor
		GpuUploadBuffer uploadBuffer(1, numBytes);
		void* dataPtr = uploadBuffer.Map();
		memcpy(dataPtr, data, numBytes);

		// Resource Transition
		// Before a heap can be the target of a GPU copy operation, the heap must first be transitioned to the D3D12_RESOURCE_STATE_COPY_DEST
		initContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		initContext.m_CommandList->CopyBufferRegion(Dest.GetResource(), destOffset, uploadBuffer.GetResource(), 0, numBytes);
		initContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		// Execute the command list and wait for it to finish so we can release the upload buffer
		initContext.Finish(true);
	}

	void CommandContext::InitializeBuffer(GpuBuffer& dest, const GpuUploadBuffer src, size_t srcOffset, size_t numBytes, size_t destOffset)
	{
		CommandContext& initContext = CommandContext::Begin();

		size_t maxBytes = std::min<size_t>(dest.GetBufferSize() - destOffset, src.GetBufferSize() - srcOffset);
		numBytes = std::min<size_t>(numBytes, maxBytes);

		// Resource Transition
		initContext.TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		initContext.m_CommandList->CopyBufferRegion(dest.GetResource(), destOffset, (ID3D12Resource*)src.GetResource(), srcOffset, numBytes);
		initContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		// Execute the command list and wait for it to finish so we can release the upload buffer
		initContext.Finish(true);
	}

	void CommandContext::InitializeTexture(GpuResource& dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA subData[])
	{
		CommandContext& initContext = CommandContext::Begin();

		UINT64 uploadBufferSize = GetRequiredIntermediateSize(dest.GetResource(), 0, NumSubresources);
		GpuUploadBuffer uploadBuffer(1, uploadBufferSize);

		UpdateSubresources(initContext.m_CommandList.Get(), dest.GetResource(), uploadBuffer.GetResource(), 0, 0, NumSubresources, subData);
		initContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ);

		initContext.Finish(true);
	}

	D3D12DynamicAllocation CommandContext::AllocateDynamicSpace(size_t numByte, size_t alignment)
	{
		return m_DynamicResourceHeap.Allocate(numByte, alignment);
	}

	void CommandContext::TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /*= false*/)
	{
		D3D12_RESOURCE_STATES OldState = resource.m_UsageState;

		// Limit the state that the Compute pipeline can be excessive
		if (m_type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			assert((OldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == OldState);
			assert((NewState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == NewState);
		}

		if (OldState != NewState)
		{
			assert(m_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
			D3D12_RESOURCE_BARRIER& barrierDesc = m_ResourceBarrierBuffer[m_numBarriersToFlush++];

			barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrierDesc.Transition.pResource = resource.GetResource();
			barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // All subresource within a resource are being transitioned
			barrierDesc.Transition.StateBefore = OldState;
			barrierDesc.Transition.StateAfter = NewState;

			// Check to see if we already started the transition
			if (NewState == resource.m_TransitioningState)
			{
				// The (sub)resource cannot be read or written by the GPU
				barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY; // Begins a split barrier and the transition barrier is said to be pending. 
				resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
			}
			else
				barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // Barrier completes the pending transition.

			resource.m_UsageState = NewState;
		}

		if(FlushImmediate || m_numBarriersToFlush == 16)
			FlushResourceBarriers();
	}

	void CommandContext::FlushResourceBarriers(void)
	{
		if (m_numBarriersToFlush > 0)
		{
			// Resource Barrier notifies the drive that need to synchronize multiple accesses to resource.
			m_CommandList->ResourceBarrier(m_numBarriersToFlush, m_ResourceBarrierBuffer);
			m_numBarriersToFlush = 0;
		}
	}
}