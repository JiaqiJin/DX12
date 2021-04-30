#include "../pch.h"
#include "CommandQueue.h"

namespace RHI
{
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device)
	{
		D3D12_COMMAND_QUEUE_DESC QueueDesc{};
		QueueDesc.Type = type;
		QueueDesc.NodeMask = 1;
		ThrowIfFailed(device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue)));
		m_CommandQueue->SetName(L"m_CommandQueue");
	}

	CommandQueue::~CommandQueue()
	{

	}
}