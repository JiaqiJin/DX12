#include "../pch.h"
#include "RenderDevice.h"

namespace RHI
{
	RenderDevice::RenderDevice(ID3D12Device* device)
		: m_device(device),
		m_CPUDescriptorHeaps
		{
			{*this, 8192, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{ *this, 2048, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },
			{ *this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE },
			{ *this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE }
		},
		m_GPUDescriptorHeaps
		{
			{*this, 16384, 32768, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE},
			{*this, 128, 1920, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE}
		},
		m_DynamicResAllocator(1, DYNAMIC_RESOURCE_PAGE_SIZE)
	{

	}

	RenderDevice::~RenderDevice()
	{
		m_DynamicResAllocator.Destroy();
	}

	DescriptorHeapAllocation RenderDevice::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1)
	{
		assert(Type >= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && Type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES && "Invalid heap type");
		return m_CPUDescriptorHeaps[Type].Allocate(Count);
	}

	DescriptorHeapAllocation RenderDevice::AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1)
	{
		assert(Type >= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && Type <= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && "Invalid heap type");
		return m_GPUDescriptorHeaps[Type].Allocate(Count);
	}

	GPUDescriptorHeap& RenderDevice::GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		assert(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		return m_GPUDescriptorHeaps[type];
	}
}