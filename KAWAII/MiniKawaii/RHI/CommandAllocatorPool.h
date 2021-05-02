#pragma once

namespace RHI
{
	/* https://milty.nl/grad_guide/basic_implementation/d3d12/command_list.html
	* A command allocator allows the app to manage the memory that is allocated for command lists. 
	* Represent GPU memory commands from CL or bundles are stored in. The memory requiered for command list is allocated by ID3D12CommandAllocator
	*/
	class CommandAllocatorPool
	{
	public:
		CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device);

		ID3D12CommandAllocator* RequestAllocator(UINT64 CompletedFenceValue);
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* allocator);

	private:
		const D3D12_COMMAND_LIST_TYPE m_cCommandListType;
		ID3D12Device* m_Device;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_Allocators;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
	};
}