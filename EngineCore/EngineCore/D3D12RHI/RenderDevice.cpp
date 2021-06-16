#include "../pch.h"
#include "RenderDevice.h"


namespace RHI
{
	RenderDevice::RenderDevice(ID3D12Device* d3d12Device)
		: m_D3D12Device(d3d12Device)
	{
		// TODO
	}

	RenderDevice::~RenderDevice()
	{
		// todo
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
}