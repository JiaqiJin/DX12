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
		D3D12_COMMAND_QUEUE_DESC QueueDesc{};
		QueueDesc.Type = type;
		QueueDesc.NodeMask = 1;
		ThrowIfFailed(device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue)));
		m_CommandQueue->SetName(L"CommandListManager::m_CommandQueue");

		// Fence
		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
		m_pFence->SetName(L"CommandListManager::m_pFence");

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

	bool CommandQueue::IsFenceComplete(UINT64 fenceValue)
	{
		// Avoid querying the fence value by testing against the last one seen.
		// The max() is to protect against an unlikely race condition that could cause the last
		// completed fence value to regress.
		if (fenceValue > m_LastCompletedFenceValue)
			m_LastCompletedFenceValue = std::max(m_LastCompletedFenceValue, m_pFence->GetCompletedValue());

		return fenceValue <= m_LastCompletedFenceValue;
	}

	void CommandQueue::WaitForFence(UINT64 fenceValue)
	{
		if (IsFenceComplete(fenceValue))
			return;

		m_pFence->SetEventOnCompletion(fenceValue, m_FenceEventHandle);
		WaitForSingleObject(m_FenceEventHandle, INFINITE);
		m_LastCompletedFenceValue = fenceValue;
	}

	UINT64 CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList)
	{
		ThrowIfFailed(((ID3D12GraphicsCommandList*)commandList)->Close());

		// Kickoff the command list
		m_CommandQueue->ExecuteCommandLists(1, &commandList);

		// Signal the next fence value (with the GPU)
		m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);

		// And increment the fence value.  
		return m_NextFenceValue++;
	}

	ID3D12CommandAllocator* CommandQueue::RequestAllocator(void)
	{
		uint64_t fenceValue = m_pFence->GetCompletedValue();

		return m_AllocatorPool.RequestAllocator(fenceValue);
	}

	void CommandQueue::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
	{
		m_AllocatorPool.DiscardAllocator(FenceValue, Allocator);
	}
}