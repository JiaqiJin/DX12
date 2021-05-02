#pragma once

#include "CommandAllocatorPool.h"

namespace RHI
{
	/*
	* Command Queue to submit the Command Lists to be excute by the GPU
	* Command Queue type : Copy, Compute(Do every thing a Copy or Dispatch), Direct(Copy and a Compute)
	*/
	class CommandQueue 
	{
		friend class CommandListManager;
		friend class CommandContext;
	public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device);
		~CommandQueue();

		UINT64 IncrementFence(void);
		bool IsFenceComplete(UINT64 FenceValue);
		void WaitForFence(UINT64 FenceValue);
		void WaitForIdle(void) { WaitForFence(IncrementFence()); }

	private:
		UINT64 ExecuteCommandList(ID3D12CommandList* commandList);

		ID3D12CommandAllocator* RequestAllocator(void);
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		// Command allocation memory
		const D3D12_COMMAND_LIST_TYPE m_Type;
		CommandAllocatorPool m_AllocatorPool;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_NextFenceValue;
		UINT64 m_LastCompletedFenceValue;
		HANDLE m_FenceEventHandle;
	};
}