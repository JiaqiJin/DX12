#pragma once

namespace RHI
{
	/* 
	* Represent the allocation of storage for GPU commands
	* A CommandList will be created when the CommandContext is created, and an Allocator will be requested at this time;
	* when the CommandList of the CommandContext is reset, an Allocator will also be requested.
	*/
	class CommandAllocatorPool
	{
	public:
		CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* Device);

		// Check the front Allocator in the Pool, if the current GPU has completed the Allocator command,
		// it can be reused, otherwise create a new one.
		ID3D12CommandAllocator* RequestAllocator(UINT64 CompletedFenceValue);

		// When the CommandContext Finish, put Allocator and the current FenceValue into the pool together
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

	private:
		const D3D12_COMMAND_LIST_TYPE m_CommandListType;

		ID3D12Device* m_Device;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_Allocators;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
	};
}