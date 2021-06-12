#include "../pch.h"
#include "GpuRenderTextureColor.h"
#include "RenderDevice.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	GpuRenderTextureColor::GpuRenderTextureColor(ID3D12Resource* resource, D3D12_RESOURCE_DESC desc, Color clearColor)
		: GpuRenderTexture(desc.Width, desc.Height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, desc.Format),
		m_ClearColor(clearColor)
	{
		m_UsageState = D3D12_RESOURCE_STATE_PRESENT;
		m_pResource.Attach(resource);
	}

	GpuRenderTextureColor::GpuRenderTextureColor(UINT32 width, UINT32 height,
		D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format, Color clearColor)
		: GpuRenderTexture(width, height, dimension, format), m_ClearColor(clearColor)
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Alignment = 0;
		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.Format = GetBaseFormat(format);
		desc.Height = (UINT)m_height;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.MipLevels = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Width = (UINT64)m_width;

		D3D12_CLEAR_VALUE ClearValue = {};
		ClearValue.Format = format;
		ClearValue.Color[0] = m_ClearColor.R();
		ClearValue.Color[1] = m_ClearColor.G();
		ClearValue.Color[2] = m_ClearColor.B();
		ClearValue.Color[3] = m_ClearColor.A();

		CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&desc, m_UsageState, &ClearValue, IID_PPV_ARGS(&m_pResource)));
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_format;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateRTV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, shared_from_this());
	
		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		RTVDesc.Format = m_format;
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
}