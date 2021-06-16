#include "../pch.h"
#include "CommandListManager.h"

namespace RHI
{
	CommandListManager::CommandListManager(ID3D12Device* device)
		: m_Device(device),
		m_GraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, device),
		m_ComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE, device),
		m_CopyQueue(D3D12_COMMAND_LIST_TYPE_COPY, device)
	{

	}

	void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type,
		ID3D12GraphicsCommandList** cmdList,
		ID3D12CommandAllocator** allocator)
	{
		assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE && "Bundles are not yet supported");

		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT: *allocator = m_GraphicsQueue.RequestAllocator();
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE: *allocator = m_ComputeQueue.RequestAllocator();
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY: *allocator = m_CopyQueue.RequestAllocator();
			break;
		default:
			break;
		}

		ThrowIfFailed(m_Device->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(cmdList)));
		(*cmdList)->SetName(L"CommandList");
	}
}