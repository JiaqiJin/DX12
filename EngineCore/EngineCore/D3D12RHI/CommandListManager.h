#pragma once
#include "CommandQueue.h"

namespace RHI
{
	/*
	* A Command List is used to issue Copy, Compute(dispatch(envio)) or draw command on GPU.
	* Beyonds the command lists, GPU adding 2 level of command list "bundles", group small number of API commands for excute later.
	*/
	class CommandListManager : public Singleton<CommandListManager>
	{
	public:
		CommandListManager(ID3D12Device* device);

		void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type,
			ID3D12GraphicsCommandList** cmdList,
			ID3D12CommandAllocator** allocator);

		CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			switch (type)
			{
				case D3D12_COMMAND_LIST_TYPE_COMPUTE: return m_ComputeQueue;
				case D3D12_COMMAND_LIST_TYPE_COPY: return m_CopyQueue;
				default: return m_GraphicsQueue;
			}
		}

		bool IsFenceComplete(uint64_t fenceValue, D3D12_COMMAND_LIST_TYPE type)
		{
			return GetQueue(type).IsFenceComplete(fenceValue);
		}

		// The CPU will wait for a fence to reach a specified value
		void WaitForFence(uint64_t fenceValue, D3D12_COMMAND_LIST_TYPE type)
		{
			return GetQueue(type).WaitForFence(fenceValue);
		}

		// The CPU will wait for all command queues to empty (so that the GPU is idle)
		void IdleGPU(void)
		{
			m_GraphicsQueue.WaitForIdle();
			m_ComputeQueue.WaitForIdle();
			m_CopyQueue.WaitForIdle();
		}

		// Getters
		CommandQueue& GetGraphicsQueue(void) { return m_GraphicsQueue; }
		CommandQueue& GetComputeQueue(void) { return m_ComputeQueue; }
		CommandQueue& GetCopyQueue(void) { return m_CopyQueue; }

	private:
		ID3D12Device* m_Device;

		CommandQueue m_GraphicsQueue;
		CommandQueue m_ComputeQueue;
		CommandQueue m_CopyQueue;
	};
}