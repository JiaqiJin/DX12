#include "../pch.h"
#include "RenderDevice.h"

namespace RHI
{
	RenderDevice::RenderDevice(ID3D12Device* device)
		: m_device(device)
	{

	}

	RenderDevice::~RenderDevice()
	{

	}
}