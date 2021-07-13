#pragma once

#include "RootSignature.h"
#include "ShaderObject/ShaderResourceLayout.h"
#include "ShaderObject/ShaderResourceCache.h"
#include "ShaderObject/ShaderVariable.h"
#include "Shader.h"
#include "ShaderObject/ShaderResourceBinding.h"
#include "ShaderObject/ShaderResourceBindingUtility.h"

namespace RHI
{
	class RenderDevice;

	// Graphic Pipeline
	struct GraphicsPipelineDesc
	{
		// Shaders Types
		std::shared_ptr<Shader> VertexShader = nullptr;
		std::shared_ptr<Shader> PixelShader = nullptr;
		std::shared_ptr<Shader> DomainShader = nullptr;
		std::shared_ptr<Shader> HullShader = nullptr;
		std::shared_ptr<Shader> GeometryShader = nullptr;
		std::shared_ptr<Shader> AmplificationShader = nullptr;
		std::shared_ptr<Shader> MeshShader = nullptr;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicPipelineState;
	};

	struct ComputePipelineDesc
	{
		Shader* ComputeShader;
	};

	struct ShaderResourceVariableDesc
	{
		SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN;
		std::string Name;
		SHADER_RESOURCE_VARIABLE_TYPE Type;
	};

	struct ShaderVariableConfig
	{
		SHADER_RESOURCE_VARIABLE_TYPE DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		std::vector<ShaderResourceVariableDesc> Variables;
	};

	struct PipelineStateDesc
	{
		std::wstring Name = L"Default PSO";

		PIPELINE_TYPE PipelineType;

		ShaderVariableConfig VariableConfig;

		GraphicsPipelineDesc GraphicsPipeline;

		ComputePipelineDesc ComputePipeline;
	};

	class PipelineState
	{
	public:
		PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc);
		~PipelineState();

		// Getters
		ID3D12PipelineState* GetD3D12PipelineState() const	{ return m_D3D12PSO.Get(); }
		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		const RootSignature* GetRootSignature() const { return &m_RootSignature; }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }
	private:
		// PSO
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;
		// Root Signature
		RootSignature m_RootSignature;
		RenderDevice* m_RenderDevice;
		PipelineStateDesc m_Desc;

		std::unordered_map<SHADER_TYPE, std::shared_ptr<Shader>> m_Shaders;
		std::unordered_map<SHADER_TYPE, ShaderResourceLayout> m_ShaderResourceLayouts;
	};
}