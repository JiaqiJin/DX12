#pragma once

#include "GpuResource.h"

namespace RHI
{
	class GpuResourceDescriptor;

	class GpuTexture : public GpuResource
	{
	public:
		GpuTexture(UINT32 width, UINT32 height, D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format)
			: m_width(width),
			m_height(height),
			m_dimension(dimension),
			m_format(format)
		{
		}

	protected:
		static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT Format);
		static size_t BytesPerPixel(DXGI_FORMAT Format);

		UINT64 m_width;
		UINT64 m_height;
		DXGI_FORMAT m_format;
		D3D12_RESOURCE_DIMENSION m_dimension;
	};
}