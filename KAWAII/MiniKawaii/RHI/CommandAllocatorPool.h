#pragma once

namespace RHI
{
	/*
	* A command allocator allows the app to manage the memory that is allocated for command lists. 
	* Represent GPU memory commands from CL or bundles are stored in.
	*/
	class CommandAllocatorPool
	{
	public:
		CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device);

	private:
		ID3D12Device* m_Device;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_allocators;
	};
}