#pragma once
#include "DescriptorHeap.h"
#include "CommandQueue.h"
#include "StaleResourceWrapper.h"
#include "CommandListManager.h"
#include "DynamicResource.h"

namespace RHI
{
	class RenderDevice : public Singleton<RenderDevice>
	{
	public:
		RenderDevice(ID3D12Device* d3d12Device);
		~RenderDevice();

		// Descriptor that allocates resources in CPU Descriptor Heap
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT Count = 1);
		// When binding resources to Shader, assign Descriptor in GPU Descriptor Heap
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT Count = 1);

		// Safely release the GPU object. It will only be released when the GPU is no longer in use. The object passed in must be moved
		template <typename DeviceObjectType>
		void SafeReleaseDeviceObject(DeviceObjectType&& object);

		void PurgeReleaseQueue(bool forceRelease);

		// Gettes
		ID3D12Device* GetD3D12Device() { return m_D3D12Device.Get(); }
		GPUDescriptorHeap& GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);
		DynamicResourceAllocator& GetDynamicResourceAllocator() { return m_DynamicResAllocator; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3D12Device;

		DynamicResourceAllocator m_DynamicResAllocator;

		// Four descriptor heap object, corresponding to the 4 descriptor heap type
		// [CBV_SRV_UAV][Sample][RTV][DSV] 
		// Each object may have multiple DX12 Descriptor Heap
		CPUDescriptorHeap  m_CPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// "2" Gpu descriptor object because only CBV_SRV_UAV and Sampler can be visible to the GPU
		// Only 1 Descriptor Heap will be created for each GPUDescriptor,because switching Descriptor Heap is very expensive
		// When creating a resource, the Descriptor of each resource is stored in the CPUDescriptorHeap, 
		// and will be copied to the GPUDescriptorHeap before the drawing command is executed
		GPUDescriptorHeap m_GPUDescriptorHeaps[2];

		// Queue responsible for releasing resource
		// When calling SafeReleaseDeviceObject to release a respirce, the resource will be added to m_StaleResources
		// When a CommandList is submitted, the number of the next CommandList and the resources in m_StaleResources will be added to m_ReleaseQueue,
		// At the end of each frame, call PurgeReleaseQueue to release resources that can be 
		// safely released (that is, all resources with a recorded Cmd number smaller than the number of CmdList completed by the GPU)
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