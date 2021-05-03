#pragma once
/*Descriptor are structure which tell shader where to find the resource, and how interprete data resource.
* Descriptor heap are chunk of memory where descriptro are stored. Shader Visible(CBV, UAV, Sampler, SRV), 
* Shader No Visible(RTV, DSV, IBV, VBV, and SOV) 
*/

namespace RHI
{
	class RenderDevice;
	
	/*
	* Descripe the allocation of the descriptors
	*/
	class DescriptorHeapAllocation
	{
	public:
		// Creates null allocation
		DescriptorHeapAllocation() noexcept :
			m_NumHandles{ 1 },
			m_pDescriptorHeap{ nullptr }
		{

		}

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCpuHandle = { 0 };
		D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGpuHandle = { 0 };
		ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;
		UINT32 m_NumHandles = 0;
	};
}