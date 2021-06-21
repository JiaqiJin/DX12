#pragma once
#include "CommandQueue.h"
#include "StaleResourceWrapper.h"
#include "CommandListManager.h"

namespace RHI
{
	class RenderDevice : public Singleton<RenderDevice>
	{
	public:
		RenderDevice(ID3D12Device* d3d12Device);
		~RenderDevice();

		// Safely release the GPU object. It will only be released when the GPU is no longer in use. The object passed in must be moved
		template <typename DeviceObjectType>
		void SafeReleaseDeviceObject(DeviceObjectType&& object);

		void PurgeReleaseQueue(bool forceRelease);

		// Gettes
		ID3D12Device* GetD3D12Device() { return m_D3D12Device.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3D12Device;

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