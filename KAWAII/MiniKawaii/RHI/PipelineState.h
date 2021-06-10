#pragma once

#include "RootSignature.h"
#include "Shader.h"
#include "ShaderResource.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "ShaderResourceBindingUtility.h"


namespace RHI
{
	class RenderDevice;
	class CommandContext;

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
		ID3D12PipelineState* GetD3D12PipelineState() const { return m_D3D12PSO.Get(); }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }
	private:
		RenderDevice* m_RenderDevice;
		// Root Signature
		RootSignature m_RootSignature;
		// PSO
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;

		PipelineStateDesc m_Desc;
	};
}