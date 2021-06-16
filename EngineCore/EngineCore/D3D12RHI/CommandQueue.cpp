#include "../pch.h"
#include "CommandQueue.h"

namespace RHI
{
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device)
		:m_Type(type),
		m_AllocatorPool(type, device),
		m_NextFenceValue(1),
		m_LastCompletedFenceValue(0)
	{
		// Command queue
		D3D12_COMMAND_QUEUE_DESC queuDesc = {};
		queuDesc.Type = type;
		queuDesc.NodeMask = 1;
		ThrowIfFailed(device->CreateCommandQueue(&queuDesc, IID_PPV_ARGS(&m_CommandQueue)));
		m_CommandQueue->SetName(L"CommandListManager::m_CommandQueue");

		// Fence
		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
		m_pFence->SetName(L"CommandListManager::m_pFence");

		// Fence Event
		m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
		assert(m_FenceEventHandle != NULL);
	}

	CommandQueue::~CommandQueue()
	{
		CloseHandle(m_FenceEventHandle);
	}

	UINT64 CommandQueue::IncrementFence(void)
	{
		m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);
		return m_NextFenceValue++;
	}

	bool CommandQueue::IsFenceComplete(UINT64 FenceValue)
	{
		// Avoid querying the fence value by testing against the last one seen.
		// The max() is to protect against an unlikely race condition that could cause the last
		// completed fence value to regress.
		if (FenceValue > m_LastCompletedFenceValue)
			m_LastCompletedFenceValue = std::max(m_LastCompletedFenceValue, m_pFence->GetCompletedValue());

		return FenceValue <= m_LastCompletedFenceValue;
	}

	void CommandQueue::WaitForFence(UINT64 FenceValue)
	{
		if (IsFenceComplete(FenceValue))
			return;

		m_pFence->SetEventOnCompletion(FenceValue, m_FenceEventHandle);
		WaitForSingleObject(m_FenceEventHandle, INFINITE);
		m_LastCompletedFenceValue = FenceValue;
	}

	UINT64 CommandQueue::ExecuteCommandList(ID3D12CommandList* List)
	{
		ThrowIfFailed(((ID3D12GraphicsCommandList*)List)->Close());

		// Kickoff the command list
		m_CommandQueue->ExecuteCommandLists(1, &List);

		// Signal the next fence value (with the GPU)
		m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);

		// And increment the fence value.  
		return m_NextFenceValue++;
	}

	ID3D12CommandAllocator* CommandQueue::RequestAllocator(void)
	{
		uint64_t CompletedFence = m_pFence->GetCompletedValue();

		return m_AllocatorPool.RequestAllocator(CompletedFence);
	}

	void CommandQueue::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator)
	{
		m_AllocatorPool.DiscardAllocator(fenceValue, allocator);
	}
}