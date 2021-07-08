#pragma once

#include "GpuResource.h"
#include "../Common/Color.h"

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
		static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT Format);
		static size_t BytesPerPixel(DXGI_FORMAT Format);

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

	class GpuRenderTextureColor : public GpuTexture
	{
	public:
		GpuRenderTextureColor(ID3D12Resource* resource, D3D12_RESOURCE_DESC desc, Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));
		GpuRenderTextureColor(UINT32 width, UINT32 height, D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format, Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));

		std::shared_ptr<GpuResourceDescriptor> CreateSRV();
		std::shared_ptr<GpuResourceDescriptor> CreateRTV();
		std::shared_ptr<GpuResourceDescriptor> CreateUAV();

		Color GetClearColor() const { return m_ClearColor; }

	protected:
		Color m_ClearColor;
	};

	class GpuRenderTextureDepth : public GpuTexture
	{
	public:
		GpuRenderTextureDepth(UINT32 width, UINT32 height, DXGI_FORMAT format, float clearDepth = 1.0f, UINT8 clearStencil = 0);

		std::shared_ptr<GpuResourceDescriptor> CreateDSV();
		std::shared_ptr<GpuResourceDescriptor> CreateDepthSRV();
		std::shared_ptr<GpuResourceDescriptor> CreateStencilSRV();

		float GetClearDepth() const { return m_ClearDepth; }
		UINT8 GetClearStencil() const { return m_ClearStencil; }
	protected:
		float m_ClearDepth;
		UINT8 m_ClearStencil;
	};
}