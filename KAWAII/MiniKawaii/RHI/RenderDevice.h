#pragma once

#include "DescriptorHeap.h"
#include "DynamicResource.h"

namespace RHI
{
	class RenderDevice : public Singleton<RenderDevice>
	{
	public:
		RenderDevice(ID3D12Device* device);
		~RenderDevice();

		// Descriptor that allocates resources in CPU descriptor heap
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
		// When binding resources to Shader, assign Descriptor in GPU Descriptor Heap
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

		template<typename DeviceObjectType>
		void SafeReleaseDeviceObject(DeviceObjectType&& object);

		// Getters
		ID3D12Device* GetD3D12Device() { return m_device.Get(); }

		GPUDescriptorHeap& GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

		DynamicResourceAllocator& GetDynamicResourceAllocator() { return m_DynamicResAllocator; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		// Four descriptor heap object, corresponding to the 4 descriptor heap type
		// [CBV_SRV_UAV][Sample][RTV][DSV] 
		// Each object may have multiple DX12 Descriptor Heap
		CPUDescriptorHeap m_CPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		// "2" Gpu descriptor object because only CBV_SRV_UAV and Sampler can be visible to the GPU
		// Only 1 Descriptor Heap will be created for each GPUDescriptor,because switching Descriptor Heap is very expensive
		// When creating a resource, the Descriptor of each resource is stored in the CPUDescriptorHeap, 
		// and will be copied to the GPUDescriptorHeap before the drawing command is executed
		GPUDescriptorHeap m_GPUDescriptorHeaps[2];

		DynamicResourceAllocator m_DynamicResAllocator;
	};


	template<typename DeviceObjectType>
	inline void RenderDevice::SafeReleaseDeviceObject(DeviceObjectType&& object)
	{
		
	}
}
