#pragma once
#include "../pch.h"
#include "CommandAllocatorPool.h"
#include <queue>
#include <mutex>
#include <stdint.h>
// http://alextardif.com/D3D11To12P1.html
namespace Rendering
{
	class CommandListManager;
	//https://docs.microsoft.com/en-us/windows/win32/direct3d12/design-philosophy-of-command-queues-and-command-lists
	class CommandQueue
	{
		friend class CommandListManager;
		friend class CommandContext;
	public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE type);
		~CommandQueue();

		void Create(ID3D12Device* pDevice);
		void ShutDown();

		inline bool IsReady()
		{
			return m_CommandQueue != nullptr;
		}

		uint64_t IncrementFence();
		bool IsFenceComplete(uint64_t fenceValue);

		void StallForFence(CommandListManager* pCmdListManager, uint64_t fenceValue);
		void StallForProducer(CommandQueue& producer);
		void WaitForFence(uint64_t fenceValue);
		void WaitForIdle() { WaitForFence(IncrementFence()); }

		ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }

		uint64_t GetNextFenceValue() { return m_NextFenceValue; }

	private:
		uint64_t ExecuteCommandList(ID3D12CommandList* list);
		ID3D12CommandAllocator* RequestAllocator();
		void DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* allocator);

	private:
		ID3D12CommandQueue* m_CommandQueue;

		const D3D12_COMMAND_LIST_TYPE m_Type;

		CommandAllocatorPool m_AllocatorPool;

		std::mutex m_FenceMutex;
		std::mutex m_EventMutex;

		// lifetime of these objects is managed by the descriptor cache
		ID3D12Fence* m_pFence;
		uint64_t m_NextFenceValue;
		uint64_t m_LastCompletedFenceValue;
		HANDLE m_FenceEventHandle;

	};
	/*
	The CommandListManager manages all the different CommandQueues: GraphicsQueue, ComputeQueue, and CopyQueue.
	*/
	class CommandListManager
	{
		friend class CommandContext;
	public:
		CommandListManager();
		~CommandListManager();
		void Create(ID3D12Device* pDevice);
		void Shutdown();

		CommandQueue& GetGraphicsQueue() { return m_GraphicsQueue; }
		CommandQueue& GetComputeQueue() { return m_ComputeQueue; }
		CommandQueue& GetCopyQueue() { return m_CopyQueue; }

		CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			switch (type)
			{
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: return m_ComputeQueue;
			case D3D12_COMMAND_LIST_TYPE_COPY: return m_CopyQueue;
			default: return m_GraphicsQueue;
			}
		}

		ID3D12CommandQueue* GetCommandQueue()
		{
			return m_GraphicsQueue.GetCommandQueue();
		}

		void CreateNewCommandList(
			D3D12_COMMAND_LIST_TYPE type,
			ID3D12GraphicsCommandList** list,
			ID3D12CommandAllocator** allocator);

		// test to see if a fence has already been reached
		bool IsFenceComplete(uint64_t fenceValue)
		{
			return GetQueue(D3D12_COMMAND_LIST_TYPE(fenceValue >> 56)).IsFenceComplete(fenceValue);
		}

		// the cpu will wait for a fence to reach a specified value
		void WaitForFence(uint64_t fenceValue);

		// the cpu will wait for all command queues to empty (so that the cpu is idle)
		void IdleGPU()
		{
			m_GraphicsQueue.WaitForIdle();
			m_ComputeQueue.WaitForIdle();
			m_CopyQueue.WaitForIdle();
		}

	private:
		ID3D12Device* m_Device;

		CommandQueue m_GraphicsQueue;
		CommandQueue m_ComputeQueue;
		CommandQueue m_CopyQueue;
	};
}