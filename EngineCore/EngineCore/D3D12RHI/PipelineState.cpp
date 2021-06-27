#include "../pch.h"
#include "PipelineState.h"
#include "RenderDevice.h"

namespace RHI
{
	PipelineState::PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc)
		: m_RenderDevice(renderDevice),
		m_RootSignature(renderDevice)
	{
		auto pd3d12Device = m_RenderDevice->GetD3D12Device();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PSODesc;
		// Status of external settings
		d3d12PSODesc.InputLayout = m_Desc.GraphicsPipeline.GraphicPipelineState.InputLayout;
		d3d12PSODesc.RasterizerState = m_Desc.GraphicsPipeline.GraphicPipelineState.RasterizerState;
		d3d12PSODesc.BlendState = m_Desc.GraphicsPipeline.GraphicPipelineState.BlendState;
		d3d12PSODesc.DepthStencilState = m_Desc.GraphicsPipeline.GraphicPipelineState.DepthStencilState;
		d3d12PSODesc.SampleMask = m_Desc.GraphicsPipeline.GraphicPipelineState.SampleMask;
		d3d12PSODesc.PrimitiveTopologyType = m_Desc.GraphicsPipeline.GraphicPipelineState.PrimitiveTopologyType;
		d3d12PSODesc.NumRenderTargets = m_Desc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets;
		for (UINT32 i = 0; i < d3d12PSODesc.NumRenderTargets; ++i)
		{
			d3d12PSODesc.RTVFormats[i] = m_Desc.GraphicsPipeline.GraphicPipelineState.RTVFormats[i];
		}
		d3d12PSODesc.SampleDesc.Count = m_Desc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Count;
		d3d12PSODesc.SampleDesc.Quality = m_Desc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Quality;
		d3d12PSODesc.DSVFormat = m_Desc.GraphicsPipeline.GraphicPipelineState.DSVFormat;

		// PSO Shader configuration
		// TODO 
		
		// Root Signature
		d3d12PSODesc.pRootSignature = m_RootSignature.GetD3D12RootSignature();

		d3d12PSODesc.NodeMask = 0;
		d3d12PSODesc.CachedPSO.pCachedBlob = nullptr;
		d3d12PSODesc.CachedPSO.CachedBlobSizeInBytes = 0;
		d3d12PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		// Creating PSO 
		ThrowIfFailed(pd3d12Device->CreateGraphicsPipelineState(&d3d12PSODesc, IID_PPV_ARGS(&m_D3D12PSO)));

		// Set name
		m_D3D12PSO->SetName(m_Desc.Name.c_str());
		std::wstring rootSignatureName(L"RootSignature For PSO ");
		rootSignatureName.append(m_Desc.Name);
		m_RootSignature.GetD3D12RootSignature()->SetName(rootSignatureName.c_str());

		// TODO
	}

	PipelineState::~PipelineState()
	{
		if (m_D3D12PSO)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3D12PSO));
	}
}