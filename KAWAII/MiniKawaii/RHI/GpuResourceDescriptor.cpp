#include "../pch.h"
#include "RenderDevice.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	GpuResourceDescriptor::GpuResourceDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, std::shared_ptr<GpuResource> OwnResource) 
		: m_Resource(OwnResource)
	{
		m_Allocation = RenderDevice::GetSingleton().AllocateDescriptor(type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GpuResourceDescriptor::GetCpuHandle() const
	{
		return m_Allocation.GetCpuHandle(0);
	}

}