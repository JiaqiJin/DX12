#include "../pch.h"
#include "CommandAllocatorPool.h"

namespace RHI
{
	CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device)
		:m_Device(device),
		m_cCommandListType(type)
	{

	}

	ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(UINT64 CompletedFenceValue)
	{
		ID3D12CommandAllocator* pAllocator = nullptr;

		return pAllocator;
	}

	void CommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* allocator)
	{
		m_ReadyAllocators.push(std::make_pair(FenceValue, allocator));
	}
}