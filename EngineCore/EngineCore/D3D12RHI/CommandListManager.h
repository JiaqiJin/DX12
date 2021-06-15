#pragma once

namespace RHI
{
	// 
	class CommandListManager : public Singleton<CommandListManager>
	{
	public:

		void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type,
			ID3D12GraphicsCommandList** List,
			ID3D12CommandAllocator** Allocator);

	private:
		ID3D12Device* m_Device;
	};
}