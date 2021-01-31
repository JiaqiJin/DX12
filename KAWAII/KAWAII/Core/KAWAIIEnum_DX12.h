#pragma once

namespace KAWAII
{
	D3D12_COMMAND_LIST_TYPE GetDX12CommandListType(CommandListType commandListType);

	D3D12_PRIMITIVE_TOPOLOGY_TYPE GetDX12PrimitiveTopologyType(PrimitiveTopologyType primitiveTopologyType);
	D3D_PRIMITIVE_TOPOLOGY GetDX12PrimitiveTopology(PrimitiveTopology primitiveTopology);

	D3D12_COMMAND_QUEUE_FLAGS GetDX12CommandQueueFlags(CommandQueueFlag commandQueueFlags);

	D3D12_CLEAR_FLAGS GetDX12ClearFlag(ClearFlag clearFlag);
	D3D12_CLEAR_FLAGS GetDX12ClearFlags(ClearFlag clearFlags);

	D3D12_FENCE_FLAGS GetDX12FenceFlag(FenceFlag fenceFlag);
	D3D12_FENCE_FLAGS GetDX12FenceFlags(FenceFlag fenceFlags);
}


