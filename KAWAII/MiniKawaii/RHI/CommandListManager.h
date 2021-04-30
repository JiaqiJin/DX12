#pragma once

namespace RHI
{
	/*
	* Command list allowed apps to record drawing or state changing "calls" on the GPU.
	* Beyonds the command lists, GPU adding 2 level of command list "bundles", group small number of API commands for excute later.
	* 
	*/
	class CommandListManager : public Singleton<CommandListManager>
	{
	public:
		CommandListManager(ID3D12Device* device);
	

	private:
		ID3D12Device* m_Device;
	};
}