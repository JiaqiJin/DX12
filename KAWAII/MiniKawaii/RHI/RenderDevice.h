#pragma once

#include "DescriptorHeap.h"
#include "CommandQueue.h"
#include "StaleResourceWrapper.h"
#include "DynamicResource.h"
#include "CommandListManager.h"


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

		void PurgeReleaseQueue(bool forceRelease);

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

		// Queue responsible for releasing resources
		// When SafeReleaseDeviceObject is called to release a resource, the resource will be added to m_StaleResources,
		// When submitting a CommandList, the number of the next CommandList and the resources in m_StaleResources will be added to m_ReleaseQueue,
		// At the end of each frame, call PurgeReleaseQueue to release resources that can be safely released 
		// (that is, all resources with a recorded Cmd number smaller than the number of CmdList completed by the GPU)
		using ReleaseQueueElementType = std::tuple<UINT64/*Graphic Queue Fence*/, StaleResourceWrapper>;
		std::deque<ReleaseQueueElementType> m_ReleaseQueue;
	};


	template<typename DeviceObjectType>
	inline void RenderDevice::SafeReleaseDeviceObject(DeviceObjectType&& object)
	{
		auto wrapper = StaleResourceWrapper::Create(object);
		uint64_t graphicNextFenceValue = CommandListManager::GetSingleton().GetGraphicsQueue().GetNextFenceValue();
		m_ReleaseQueue.emplace_back(graphicNextFenceValue, std::move(wrapper));
	}
}
