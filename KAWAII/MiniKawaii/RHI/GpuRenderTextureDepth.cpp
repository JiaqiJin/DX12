#include "../pch.h"
#include "GpuRenderTextureDepth.h"
#include "RenderDevice.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	GpuRenderTextureDepth::GpuRenderTextureDepth(UINT32 width, UINT32 height, DXGI_FORMAT format, float clearDepth, UINT8 clearStencil) 
		: GpuRenderTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format),
		m_ClearDepth(clearDepth),
		m_ClearStencil(clearStencil)
	{
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		resourceDesc.Format = GetBaseFormat(format);
		resourceDesc.Height = (UINT)m_height;
		resourceDesc.Width = (UINT64)m_width;
		resourceDesc.Alignment = 0;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 0;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_CLEAR_VALUE ClearValue{};
		// ID3D12Device::CreateCommittedResource: D3D12_CLEAR_VALUE::Format cannot be a typeless format. A fully qualified format must be supplied
		ClearValue.DepthStencil.Depth = m_ClearDepth;
		ClearValue.DepthStencil.Stencil = m_ClearStencil;
		ClearValue.Format = GetDSVFormat(format);

		CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, m_UsageState, &ClearValue, IID_PPV_ARGS(&m_pResource)));
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureDepth::CreateDSV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, shared_from_this());

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = GetDSVFormat(m_format);
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateDepthStencilView(m_pResource.Get(), &dsvDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureDepth::CreateDepthSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = GetDepthFormat(m_format);
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureDepth::CreateStencilSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = GetStencilFormat(m_format);
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}
}