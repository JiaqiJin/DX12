#include "../pch.h"
#include "ShaderResourceLayout.h"

namespace RHI
{
	ShaderResourceLayout::ShaderResourceLayout(ID3D12Device* pd3d12Device,
		PIPELINE_TYPE pipelineType,
		const ShaderVariableConfig& shaderVariableConfig,
		const ShaderResource* shaderResource,
		RootSignature* rootSignature) :
		m_D3D12Device(pd3d12Device)
	{

	}
}