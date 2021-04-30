#pragma once

namespace RHI
{
	class CommandQueue 
	{
	public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device);
		~CommandQueue();

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	};
}