#pragma once

#include "RootSignature.h"
#include "Shader.h"
#include "ShaderVariable.h"
#include "ShaderResourceBinding.h"
#include "ShaderResource.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "ShaderResourceBindingUtility.h"


namespace RHI
{
	class RenderDevice;
	class CommandContext;

	// Graphic pipeline
	struct GraphicsPipelineDesc
	{
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
		std::string name;
		SHADER_RESOURCE_VARIABLE_TYPE Type;
	};

	// Describe the configuration of ShaderVariableType in the PSO, the upper layer can specify the type of ShaderVariable,
	// and the default is DefaultVariableType
	struct ShaderVariableConfig
	{
		SHADER_RESOURCE_VARIABLE_TYPE DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		std::vector<ShaderResourceVariableDesc> Variables;
	};

	// Descipe Pipeline State
	struct PipelineStateDesc
	{
		std::wstring Name = L"Default PSO";

		PIPELINE_TYPE PipelineType;

		ShaderVariableConfig VariableConfig;

		GraphicsPipelineDesc GraphicsPipeline;

		ComputePipelineDesc ComputePipeline;
	};

	/*
	* Hardware setting that determine how the input data is interpreted and rendered (Setting : rasterizer state, blend state etc)
	*/
	class PipelineState
	{
		friend class CommandContext;
	public:
		PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc);
		~PipelineState();

		// GETTERS
		const PipelineStateDesc& Get() const { return m_Desc; }

		UINT32 GetStaticVariableCount(SHADER_TYPE ShaderType) const;
		ShaderVariable* GetStaticVariableByName(SHADER_TYPE shaderType, std::string name);
		ShaderVariable* GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index);

		// Create SRB, the application binds Mutable and Dynamic resources through SRB, SRB object is owned by PSO
		std::unique_ptr<ShaderResourceBinding> CreateShaderResourceBinding();

		template<typename TOperation>
		void ProcessShaders(TOperation Operation) const
		{
			for (const auto& [shaderType, shader] : m_Shaders)
			{
				Operation(shaderType, m_ShaderResourceLayouts.at(shaderType));
			}
		}

		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		const RootSignature* GetRootSignature() const { return &m_RootSignature; }

		ID3D12PipelineState* GetD3D12PipelineState() const { return m_D3D12PSO.Get(); }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }
	private:
		RenderDevice* m_RenderDevice;
		// Root Signature
		RootSignature m_RootSignature;
		// PSO
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;

		PipelineStateDesc m_Desc;

		// Using shared_ptr, a Shader may be shared by multiple PSOs
		std::unordered_map<SHADER_TYPE, std::shared_ptr<Shader>> m_Shaders;
		std::unordered_map<SHADER_TYPE, ShaderResourceLayout> m_ShaderResourceLayouts;

		std::unique_ptr<ShaderResourceBinding> m_StaticSRB;
	};
}