#pragma once

namespace RHI
{
	/*
	* Command list allowed apps to record drawing or state changing "calls" on the GPU. we use CL to allocate command to excute in GPU
	* Beyonds the command lists, GPU adding 2 level of command list "bundles", group small number of API commands for excute later.
	* 
	*/
	class CommandListManager : public Singleton<CommandListManager>
	{
	public:
		CommandListManager(ID3D12Device* device);
	
		void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** cmdList, ID3D12CommandAllocator** allocator);
	private:
		ID3D12Device* m_Device;
	};
}