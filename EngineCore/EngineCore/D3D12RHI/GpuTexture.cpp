#include "../pch.h"
#include "GpuTexture.h"
#include "GpuResourceDescriptor.h"
#include "RenderDevice.h"
#include "CommandContext.h"

namespace RHI
{

	// ----------------------- TEXTURE --------------------------------
	DXGI_FORMAT GpuTexture::GetBaseFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_TYPELESS;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_TYPELESS;

			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32G8X24_TYPELESS;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_TYPELESS;

		default:
			return defaultFormat;
		}
	}

	DXGI_FORMAT GpuTexture::GetUAVFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_UNORM;

		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;

#ifdef _DEBUG
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_D16_UNORM:

			assert(false && "Requested a UAV Format for a depth stencil Format.");
#endif

		default:
			return defaultFormat;
		}
	}

	// The X in these formats means Padding, for example, DXGI_FORMAT_D32_FLOAT_S8X24_UINT is a 32-bit depth, 8-bit template,
	// and 24-bit for Padding
	DXGI_FORMAT GpuTexture::GetDSVFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_D16_UNORM;

		default:
			return defaultFormat;
		}
	}

	DXGI_FORMAT GpuTexture::GetDepthFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT GpuTexture::GetStencilFormat(DXGI_FORMAT defaultFormat)
	{
		switch (defaultFormat)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	size_t GpuTexture::BytesPerPixel(DXGI_FORMAT Format)
	{
		switch (Format)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 16;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 12;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 8;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return 4;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return 2;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_P8:
			return 1;

		default:
			return 0;
		}
	}

	// ----------------------- TEXTURE 2D ------------------------
	GpuTexture2D::GpuTexture2D(UINT32 width, UINT32 height, DXGI_FORMAT format, UINT64 RowPitchBytes, const void* InitialData)
		: GpuTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format)
	{
		m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = (UINT)height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		D3D12_SUBRESOURCE_DATA texResource;
		texResource.pData = InitialData;
		texResource.RowPitch = RowPitchBytes;
		texResource.SlicePitch = RowPitchBytes * height;

		CommandContext::InitializeTexture(*this, 1, &texResource);
	}

	std::shared_ptr<GpuResourceDescriptor> GpuTexture2D::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_Format;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	 // ---------------------- Color Texture ---------------------------
	GpuRenderTextureColor::GpuRenderTextureColor(ID3D12Resource* resource, D3D12_RESOURCE_DESC desc, Color clearColor)
		: GpuTexture((UINT32)desc.Width, (UINT32)desc.Height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, desc.Format),
		m_ClearColor(clearColor)
	{
		m_UsageState = D3D12_RESOURCE_STATE_PRESENT;
		m_pResource.Attach(resource);
	}

	GpuRenderTextureColor::GpuRenderTextureColor(UINT32 width, UINT32 height, 
		D3D12_RESOURCE_DIMENSION dimension,
		DXGI_FORMAT format, 
		Color clearColor)
		: GpuTexture(width, height, dimension, format),
		m_ClearColor(clearColor)
	{
		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		Desc.Format = GetBaseFormat(format);
		Desc.Height = (UINT)m_Height;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		Desc.MipLevels = 0;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_Width;

		D3D12_CLEAR_VALUE ClearValue = {};
		ClearValue.Format = format;
		ClearValue.Color[0] = m_ClearColor.R();
		ClearValue.Color[1] = m_ClearColor.G();
		ClearValue.Color[2] = m_ClearColor.B();
		ClearValue.Color[3] = m_ClearColor.A();

		CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&Desc, m_UsageState, &ClearValue, IID_PPV_ARGS(&m_pResource)));
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_Format;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateRTV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, shared_from_this());

		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		RTVDesc.Format = m_Format;
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateRenderTargetView(m_pResource.Get(), &RTVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateUAV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		// TODO: Implete UAV

		return descriptor;
	}

	// ---------------------------- DEPTH STENCIL TEXTURE -------------------------
	GpuRenderTextureDepth::GpuRenderTextureDepth(UINT32 width, UINT32 height, DXGI_FORMAT format, float clearDepth, UINT8 clearStencil) :
		GpuTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format),
		m_ClearDepth(clearDepth),
		m_ClearStencil(clearStencil)
	{
		D3D12_RESOURCE_DESC Desc = {};
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		Desc.Format = GetBaseFormat(format);
		Desc.Height = (UINT)m_Height;
		Desc.Width = (UINT64)m_Width;
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.MipLevels = 0;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_CLEAR_VALUE ClearValue = {};
		// ID3D12Device::CreateCommittedResource: D3D12_CLEAR_VALUE::Format cannot be a typeless format. A fully qualified format must be supplied
		ClearValue.Format = GetDSVFormat(format);
		ClearValue.DepthStencil.Depth = m_ClearDepth;
		ClearValue.DepthStencil.Stencil = m_ClearStencil;

		CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&Desc, m_UsageState, &ClearValue, IID_PPV_ARGS(&m_pResource)));
	}

	std::shared_ptr<RHI::GpuResourceDescriptor> GpuRenderTextureDepth::CreateDSV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, shared_from_this());

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = GetDSVFormat(m_Format);
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateDepthStencilView(m_pResource.Get(), &dsvDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<RHI::GpuResourceDescriptor> GpuRenderTextureDepth::CreateDepthSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = GetDepthFormat(m_Format);
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<RHI::GpuResourceDescriptor> GpuRenderTextureDepth::CreateStencilSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = GetStencilFormat(m_Format);
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

}