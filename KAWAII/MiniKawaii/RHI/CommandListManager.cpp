#include "../pch.h"
#include "CommandListManager.h"
#include "RenderDevice.h"

namespace RHI
{
	CommandListManager::CommandListManager(ID3D12Device* device)
		: m_Device(device)
	{

	}

	void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type,
		ID3D12GraphicsCommandList** cmdList, ID3D12CommandAllocator** allocator) // TODO
	{
		assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE && "Bundles are not yet supported");

		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			break;
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			break;
		case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
			break;
		case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
			break;
		case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
			break;
		default:
			break;
		}

		ThrowIfFailed(m_Device->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(cmdList)));
		(*cmdList)->SetName(L"CommandList");
	}

}