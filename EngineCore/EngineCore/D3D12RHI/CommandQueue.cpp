#include "../pch.h"
#include "CommandQueue.h"

namespace RHI
{
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* device)
		:m_Type(Type), 
		m_AllocatorPool(Type, device)
	{

	}

	CommandQueue::~CommandQueue()
	{

	}
}