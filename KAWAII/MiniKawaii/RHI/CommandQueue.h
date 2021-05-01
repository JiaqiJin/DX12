#pragma once

namespace RHI
{
	/*
	* Command Queue to submit the Command Lists to be excute by the GPU
	*/
	class CommandQueue 
	{
	public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device);
		~CommandQueue();

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	};
}