#pragma once

#include "GpuTexture.h"
// https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-textures-from-file
namespace RHI
{
	class GpuTexture2D : public GpuTexture
	{
	public:
		GpuTexture2D(UINT32 width, UINT32 height, DXGI_FORMAT format, UINT64 rowPitchBytes, const void* initialData);

		std::shared_ptr<GpuResourceDescriptor> CreateSRV();
	};
}