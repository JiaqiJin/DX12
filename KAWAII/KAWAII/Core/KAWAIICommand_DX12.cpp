#include "KAWAIICommand_DX12.h"
#include "KAWAIIEnum_DX12.h"

using namespace std;
using namespace KAWAII;

bool DX12Device::GetCommandQueue(CommandQueue& commandQueue, CommandListType type, CommandQueueFlag flags, int32_t priority, uint32_t nodeMask)
{
	return false;
}

bool DX12Device::GetCommandAllocator(CommandAllocator& commandAllocator, CommandListType type)
{
	return false;
}

bool DX12Device::GetCommandList(CommandList* pCommandList, uint32_t nodeMask, CommandListType type,
	const CommandAllocator& commandAllocator, const Pipeline& pipeline)
{
	return false;
}

bool DX12Device::GetCommandList(CommandList& commandList, uint32_t nodeMask, CommandListType type,
	const CommandAllocator& commandAllocator, const Pipeline& pipeline)
{
	return false;
}

bool DX12Device::GetFence(Fence& fence, uint64_t initialValue, FenceFlag flags)
{
	return false;
}

bool DX12Device::CreateCommandLayout()
{
	return false;
}