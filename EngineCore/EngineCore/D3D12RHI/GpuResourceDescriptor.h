#pragma once
#include "GpuResource.h"
#include "DescriptorHeap.h"

namespace RHI
{
	/*
	* Represents the Descriptor or RTV, DSV of the resource. It is allocated in the CPUDescriptorHeap
	*/
	class GpuResourceDescriptor 
	{
	public:
		GpuResourceDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, std::shared_ptr<GpuResource> OwnResource);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const;

		const GpuResource* GetResource() const { return m_Resource.get(); }
		bool IsNull() const { return m_Allocation.IsNull(); }
		bool IsShaderVisible() const { return m_Allocation.IsShaderVisible(); }
	private:
		std::shared_ptr<GpuResource> m_Resource;

		DescriptorHeapAllocation m_Allocation;
	};
}
