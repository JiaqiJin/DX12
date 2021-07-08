#include "../pch.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "RenderDevice.h"
#include "DescriptorHeap.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "GpuTexture.h"
#include "DynamicResource.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	CommandContext* CommandContextManager::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type)
	{
		CommandContext* context = nullptr;

		auto& avaliableCommandContexts = m_AvailableCommandContexts[type];

		if (avaliableCommandContexts.empty())
		{
			context = new CommandContext(type);
			m_CommandContextPool[type].emplace_back(context);
			context->Initialize();
		}
		else
		{
			context = avaliableCommandContexts.front();
			avaliableCommandContexts.pop();
			context->Reset();
		}

		assert(context != nullptr);
		assert(context->m_Type == type);

		return context;
	}

	void CommandContextManager::FreeCommandContext(CommandContext* usedContext)
	{
		assert(usedContext != nullptr);
		m_AvailableCommandContexts[usedContext->m_Type].push(usedContext);
	}

	// ------------------- CommandContext ------------------------------

	CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
		: m_Type(type),
		m_CommandList(nullptr),
		m_CurrentAllocator(nullptr),
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
		// Creating the new command list
		CommandListManager::GetSingleton().CreateNewCommandList(m_Type, m_CommandList.GetAddressOf(), &m_CurrentAllocator);
	}

	void CommandContext::Reset()
	{
		assert(m_CommandList != nullptr && m_CurrentAllocator == nullptr);

		m_CurrentAllocator = CommandListManager::GetSingleton().GetQueue(m_Type).RequestAllocator();
		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO
	}

	CommandContext& CommandContext::Begin(const std::wstring ID)
	{
		CommandContext* newContext = CommandContextManager::GetSingleton().AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		newContext->SetID(ID);

		return *newContext;
	}

	uint64_t CommandContext::Finish(bool WaitForCompletion, bool releaseDynamic)
	{
		assert(m_Type == D3D12_COMMAND_LIST_TYPE_DIRECT || m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE);

		FlushResourceBarriers();

		assert(m_CurrentAllocator != nullptr);

		CommandQueue& Queue = CommandListManager::GetSingleton().GetQueue(m_Type);

		// Clean Release Queue
		RenderDevice::GetSingleton().PurgeReleaseQueue(false);

		// Release dynamic resources at the end of each frame
		if (releaseDynamic)
		{
			m_DynamicGPUDescriptorAllocator.ReleaseAllocations();

			m_DynamicResourceHeap.ReleaseAllocatedPages();
		}

		uint64_t FenceValue = Queue.ExecuteCommandList(m_CommandList.Get());
		Queue.DiscardAllocator(FenceValue, m_CurrentAllocator);
		m_CurrentAllocator = nullptr;


		if (WaitForCompletion)
			CommandListManager::GetSingleton().WaitForFence(FenceValue, m_Type);

		CommandContextManager::GetSingleton().FreeCommandContext(this);

		return FenceValue;
	}


	// Resource Initialization
	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset)
	{
		CommandContext& InitContext = CommandContext::Begin();

		// Copy to UploadBuffer, the UploadBuffer here will be automatically released, and SafeRelease will be called in the destructor
		GpuUploadBuffer uploadBuffer(1, (UINT32)NumBytes);
		void* dataPtr = uploadBuffer.Map();
		memcpy(dataPtr, Data, NumBytes);

		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		InitContext.m_CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, uploadBuffer.GetResource(), 0, NumBytes);
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		// Excute the Command List and wait it to finish so we can release the upload buffer
		InitContext.Finish(true);
	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const GpuUploadBuffer& Src, size_t SrcOffset, size_t NumBytes, size_t DestOffset)
	{
		CommandContext& InitContext = CommandContext::Begin();

		size_t MaxBytes = std::min<size_t>(Dest.GetBufferSize() - DestOffset, Src.GetBufferSize() - SrcOffset);
		NumBytes = std::min<size_t>(MaxBytes, NumBytes);

		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		InitContext.m_CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, (ID3D12Resource*)Src.GetResource(), SrcOffset, NumBytes);
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);
		// Execute the command list and wait for it to finish so we can release the upload buffer
		InitContext.Finish(true);
	}

	void CommandContext::InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[])
	{
		CommandContext& InitContext = CommandContext::Begin();

		UINT64 uploadBufferSize = GetRequiredIntermediateSize(Dest.GetResource(), 0, NumSubresources);
		GpuUploadBuffer uploadBuffer(1, (UINT32)uploadBufferSize);

		UpdateSubresources(InitContext.m_CommandList.Get(), Dest.GetResource(), uploadBuffer.GetResource(), 0, 0, NumSubresources, SubData);
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ);

		// Execute the command list and wait for it to finish so we can release the upload buffer
		InitContext.Finish(true);
	}

	void CommandContext::TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /*= false*/)
	{
		// TODO
	}

	void CommandContext::FlushResourceBarriers()
	{
		// TODO
	}

	D3D12DynamicAllocation CommandContext::AllocateDynamicSpace(size_t NumBytes, size_t Alignment)
	{
		return m_DynamicResourceHeap.Allocate(NumBytes, Alignment);
	}

	// ------------------ Graphic Context ---------------------
	void GraphicsContext::ClearColor(GpuResourceDescriptor& RTV, D3D12_RECT* Rect /*= nullptr*/)
	{
		FlushResourceBarriers();

		const GpuResource* resource = RTV.GetResource();
		const GpuRenderTextureColor* rt = dynamic_cast<const GpuRenderTextureColor*>(resource);
		if (rt != nullptr)
			m_CommandList->ClearRenderTargetView(RTV.GetCpuHandle(), rt->GetClearColor().GetPtr(), (Rect == nullptr) ? 0 : 1, Rect);
	}

	void GraphicsContext::ClearColor(GpuResourceDescriptor& RTV, Color Colour, D3D12_RECT* Rect /*= nullptr*/)
	{
		FlushResourceBarriers();

		const GpuResource* resource = RTV.GetResource();
		const GpuRenderTextureColor* rt = dynamic_cast<const GpuRenderTextureColor*>(resource);
		if (rt != nullptr)
			m_CommandList->ClearRenderTargetView(RTV.GetCpuHandle(), Colour.GetPtr(), (Rect == nullptr) ? 0 : 1, Rect);
	}

	void GraphicsContext::ClearDepth(GpuResourceDescriptor& DSV)
	{
		FlushResourceBarriers();

		const GpuResource* resource = DSV.GetResource();
		const GpuRenderTextureDepth* depth = dynamic_cast<const GpuRenderTextureDepth*>(resource);
		if (depth != nullptr)
			m_CommandList->ClearDepthStencilView(DSV.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH, depth->GetClearDepth(), depth->GetClearStencil(), 0, nullptr);
	}

	void GraphicsContext::ClearStencil(GpuResourceDescriptor& DSV)
	{
		FlushResourceBarriers();

		const GpuResource* resource = DSV.GetResource();
		const GpuRenderTextureDepth* depth = dynamic_cast<const GpuRenderTextureDepth*>(resource);
		if (depth != nullptr)
			m_CommandList->ClearDepthStencilView(DSV.GetCpuHandle(), D3D12_CLEAR_FLAG_STENCIL, depth->GetClearDepth(), depth->GetClearStencil(), 0, nullptr);
	}

	void GraphicsContext::ClearDepthAndStencil(GpuResourceDescriptor& DSV)
	{
		FlushResourceBarriers();

		const GpuResource* resource = DSV.GetResource();
		const GpuRenderTextureDepth* depth = dynamic_cast<const GpuRenderTextureDepth*>(resource);
		if (depth != nullptr)
			m_CommandList->ClearDepthStencilView(DSV.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth->GetClearDepth(), depth->GetClearStencil(), 0, nullptr);
	}

	void GraphicsContext::SetViewport(const D3D12_VIEWPORT& vp)
	{
		m_CommandList->RSSetViewports(1, &vp);
	}

	void GraphicsContext::SetScissor(const D3D12_RECT& rect)
	{
		assert(rect.left < rect.right&& rect.top < rect.bottom);
		m_CommandList->RSSetScissorRects(1, &rect);
	}

	void GraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
	{
		m_CommandList->IASetPrimitiveTopology(Topology);
	}

	void GraphicsContext::SetRenderTargets(UINT NumRTVs, GpuResourceDescriptor* RTVs[], GpuResourceDescriptor* DSV /*= nullptr*/)
	{
		std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> RTVHandles = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle;

		if (NumRTVs > 0)
		{
			RTVHandles.reset(new D3D12_CPU_DESCRIPTOR_HANDLE[NumRTVs]);
			for (INT32 i = 0; i < NumRTVs; ++i)
			{
				RTVHandles[i] = RTVs[i]->GetCpuHandle();
			}
		}

		if (DSV != nullptr)
		{
			DSVHandle = DSV->GetCpuHandle();
			m_CommandList->OMSetRenderTargets(NumRTVs, RTVHandles.get(), FALSE, &DSVHandle);
		}
		else
		{
			m_CommandList->OMSetRenderTargets(NumRTVs, RTVHandles.get(), FALSE, nullptr);
		}
	}

	void GraphicsContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView)
	{
		m_CommandList->IASetVertexBuffers(Slot, 1, &VBView);
	}

	void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView)
	{
		m_CommandList->IASetIndexBuffer(&IBView);
	}

	void GraphicsContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferAddress)
	{
		m_CommandList->SetGraphicsRootConstantBufferView(RootIndex, BufferAddress);
	}

	void GraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable)
	{
		m_CommandList->SetGraphicsRootDescriptorTable(RootIndex, DescriptorTable);
	}

	void GraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset /*= 0*/)
	{
		DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
	}

	void GraphicsContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation /*= 0*/, INT BaseVertexLocation /*= 0*/)
	{
		DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
	}

	void GraphicsContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation /*= 0*/, UINT StartInstanceLocation /*= 0*/)
	{
		// TODO
	}

	void GraphicsContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		// TODO
	}


}