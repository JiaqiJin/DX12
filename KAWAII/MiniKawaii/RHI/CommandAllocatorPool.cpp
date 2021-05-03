#include "../pch.h"
#include "CommandAllocatorPool.h"

namespace RHI
{
	CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device)
		:m_Device(device),
		m_cCommandListType(type)
	{

	}

	ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(UINT64 completedFenceValue)
	{
		ID3D12CommandAllocator* pAllocator = nullptr;

		if (!m_ReadyAllocators.empty())
		{
			std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocators.front();

			if (AllocatorPair.first <= completedFenceValue)
			{
				pAllocator = AllocatorPair.second;
				ThrowIfFailed(pAllocator->Reset());
				m_ReadyAllocators.pop();
			}
		}

		// If no allocator's were ready to be reused, create a new one
		if (pAllocator == nullptr)
		{
			ThrowIfFailed(m_Device->CreateCommandAllocator(m_cCommandListType, IID_PPV_ARGS(&pAllocator)));
			wchar_t allocatorName[32];
			swprintf(allocatorName, 32, L"CommandAllocator %zu", m_Allocators.size());
			pAllocator->SetName(allocatorName);
			m_Allocators.push_back(pAllocator);
		}

		return pAllocator;
	}

	void CommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* allocator)
	{
		m_ReadyAllocators.push(std::make_pair(FenceValue, allocator));
	}
}