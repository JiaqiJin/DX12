#pragma once

#include "GpuResource.h"

namespace RHI
{
	class GpuResourceDescriptor;

	class GpuTexture : public GpuResource
	{
	public:
		GpuTexture(UINT32 width, UINT32 height, D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format) :
			m_Width(width),
			m_Height(height),
			m_Dimension(dimension),
			m_Format(format)
		{

		}

	protected:
		UINT64 m_Width;
		UINT64 m_Height;
		DXGI_FORMAT m_Format;
		D3D12_RESOURCE_DIMENSION m_Dimension;
	};

	// Texture 2D
	class GpuTexture2D : public GpuTexture
	{
	public:
		GpuTexture2D(UINT32 width, UINT32 height, DXGI_FORMAT format, UINT64 RowPitchBytes, const void* InitialData);

		std::shared_ptr<GpuResourceDescriptor> CreateSRV();
	};
}