#pragma once
#include "CommandAllocatorPool.h"

namespace RHI
{
	class CommandQueue
	{
		friend class CommandListManager;
		friend class CommandContext;
	public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* device);
		~CommandQueue();

		UINT64 IncrementFence(void);
		bool IsFenceComplete(UINT64 FenceValue);
		void WaitForFence(UINT64 FenceValue);
		void WaitForIdle(void) { WaitForFence(IncrementFence()); }

		UINT64 GetNextFenceValue() const { return m_NextFenceValue; }
		UINT64 GetCompletedFenceValue() const { return m_pFence->GetCompletedValue(); }
	private:
		UINT64 ExecuteCommandList(ID3D12CommandList* List);

		ID3D12CommandAllocator* RequestAllocator(void);
		void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator);

		// CommandQueue
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		
		const D3D12_COMMAND_LIST_TYPE m_Type;
		// CommandPool 
		CommandAllocatorPool m_AllocatorPool;

		// Fences
		Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_NextFenceValue;
		UINT64 m_LastCompletedFenceValue;
		HANDLE m_FenceEventHandle;
	};
}