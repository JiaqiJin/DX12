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

		if(m_Desc.PipelineType == PIPELINE_TYPE_GRAPHIC)
		{
			auto addShader = [&](std::shared_ptr<Shader> shader)
			{
				if(shader != nullptr)
					m_Shaders.insert(std::make_pair(shader->GetShaderType(), shader));
			};

			addShader(m_Desc.GraphicsPipeline.VertexShader);
			addShader(m_Desc.GraphicsPipeline.PixelShader);
			addShader(m_Desc.GraphicsPipeline.GeometryShader);
			addShader(m_Desc.GraphicsPipeline.HullShader);
			addShader(m_Desc.GraphicsPipeline.DomainShader);

			assert(m_Shaders.size() > 0 && "No shader in current pipeline");

			for (const auto& [shaderType, shader] : m_Shaders)
			{
				m_ShaderResourceLayouts.insert(std::make_pair(shaderType, ShaderResourceLayout(pd3d12Device,
					m_Desc.PipelineType,
					m_Desc.VariableConfig,
					shader->GetShaderResources(),
					&m_RootSignature)));
			}

			// The root signature is initialized and the RootSignature of Direct3D 12 is created
			m_RootSignature.Finalize(pd3d12Device);

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
			for (const auto& [shaderType, shader] : m_Shaders)
			{
				switch (shaderType)
				{
				case SHADER_TYPE_VERTEX:
					d3d12PSODesc.VS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.VS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_PIXEL:
					d3d12PSODesc.PS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.PS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_GEOMETRY:
					d3d12PSODesc.GS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.GS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_HULL:
					d3d12PSODesc.HS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.HS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_DOMAIN:
					d3d12PSODesc.DS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.DS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				default:
					LOG_ERROR("UnExpected Shader Type.");
					break;
				}
			}

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

			//TODO
		}

		// TODO Compute Pipeline
	}

	PipelineState::~PipelineState()
	{
		if (m_D3D12PSO)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3D12PSO));
	}
}