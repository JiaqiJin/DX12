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

	private:
		// CommandQueue
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		
		const D3D12_COMMAND_LIST_TYPE m_Type;
		// CommandPool 
		CommandAllocatorPool m_AllocatorPool;
	};
}