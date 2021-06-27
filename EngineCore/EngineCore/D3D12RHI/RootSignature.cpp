#include "../pch.h"
#include "RootSignature.h"
#include "RenderDevice.h"

namespace RHI
{
	RootSignature::RootSignature(RenderDevice* renderDevice)
		:m_RenderDevice(renderDevice)
	{
		// TODO
	}

	RootSignature::~RootSignature()
	{
		if (m_pd3d12RootSignature)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_pd3d12RootSignature));
	}

}