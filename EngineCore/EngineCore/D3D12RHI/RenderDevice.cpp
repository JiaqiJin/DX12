#include "../pch.h"
#include "RenderDevice.h"


namespace RHI
{
	RenderDevice::RenderDevice(ID3D12Device* d3d12Device)
		: m_D3D12Device(d3d12Device),
		m_CPUDescriptorHeaps
		{
			{*this, 8192, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{*this, 2048, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{*this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{*this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE}
		},
		m_GPUDescriptorHeaps
		{
			{*this, 16384, 32768, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE},
			{*this, 128, 1920, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE}
		}
	{
		// TODO
	}

	RenderDevice::~RenderDevice()
	{
		// todo
	}

	// Descriptor that allocates resources in CPU Descriptor Heap
	DescriptorHeapAllocation RenderDevice::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT Count)
	{
		assert(type >= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES && "Invalid heap type");
		return m_CPUDescriptorHeaps[type].Allocate(Count);
	}

	// When binding resources to Shader, assign Descriptor in GPU Descriptor Heap
	DescriptorHeapAllocation RenderDevice::AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT Count)
	{
		assert(type >= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && type <= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && "Invalid heap type");
		return m_GPUDescriptorHeaps[type].Allocate(Count);
	}

	void RenderDevice::PurgeReleaseQueue(bool forceRelease)
	{
		UINT64 graphicCompletedFenceValue = CommandListManager::GetSingleton().GetGraphicsQueue().GetCompletedFenceValue();

		if (forceRelease)
		{
			graphicCompletedFenceValue = std::numeric_limits<UINT64>::max();
			//computeCompletedFenceValue = std::numeric_limits<UINT64>::max();
			//copyCompletedFenceValue = std::numeric_limits<UINT64>::max();
		}

		while (!m_ReleaseQueue.empty())
		{
			auto& firstObj = m_ReleaseQueue.front();
			if (std::get<0>(firstObj) <= graphicCompletedFenceValue)
				m_ReleaseQueue.pop_front();
			else
				break;
		}
	}

	GPUDescriptorHeap& RenderDevice::GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
	{
		assert(Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		return m_GPUDescriptorHeaps[Type];
	}
}