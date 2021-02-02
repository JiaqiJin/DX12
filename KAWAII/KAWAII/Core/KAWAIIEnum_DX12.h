#pragma once

namespace KAWAII
{
	D3D12_COMMAND_LIST_TYPE GetDX12CommandListType(CommandListType commandListType);
	D3D12_HEAP_TYPE GetDX12HeapType(MemoryType memoryType);
	D3D12_PRIMITIVE_TOPOLOGY_TYPE GetDX12PrimitiveTopologyType(PrimitiveTopologyType primitiveTopologyType);
	D3D_PRIMITIVE_TOPOLOGY GetDX12PrimitiveTopology(PrimitiveTopology primitiveTopology);

	D3D12_COMMAND_QUEUE_FLAGS GetDX12CommandQueueFlags(CommandQueueFlag commandQueueFlags);

	D3D12_RESOURCE_FLAGS GetDX12ResourceFlag(ResourceFlag resourceFlag);
	D3D12_RESOURCE_FLAGS GetDX12ResourceFlags(ResourceFlag resourceFlags);


	D3D12_CLEAR_FLAGS GetDX12ClearFlag(ClearFlag clearFlag);
	D3D12_CLEAR_FLAGS GetDX12ClearFlags(ClearFlag clearFlags);

	D3D12_RESOURCE_STATES GetDX12ResourceState(ResourceState resourceState);
	D3D12_RESOURCE_STATES GetDX12ResourceStates(ResourceState resourceStates);

	D3D12_RESOURCE_BARRIER_FLAGS GetDX12BarrierFlag(BarrierFlag barrierFlag);
	D3D12_RESOURCE_BARRIER_FLAGS GetDX12BarrierFlags(BarrierFlag barrierFlags);

	D3D12_FENCE_FLAGS GetDX12FenceFlag(FenceFlag fenceFlag);
	D3D12_FENCE_FLAGS GetDX12FenceFlags(FenceFlag fenceFlags);
}



