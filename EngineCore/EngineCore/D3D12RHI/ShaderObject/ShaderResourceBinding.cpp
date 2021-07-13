#include "../../pch.h"
#include "ShaderResourceBinding.h"
#include "../PipelineState.h"
#include "../RenderDevice.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
	ShaderResourceBinding::ShaderResourceBinding(PipelineState* PSO,
		const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
		UINT32 allowedTypeNum) :
		m_PSO{ PSO }
	{
		auto* renderDevice = PSO->GetRenderDevice();

		m_ShaderResourceCache.Initialize(renderDevice, m_PSO->GetRootSignature(), allowedVarTypes, allowedTypeNum);

		m_PSO->ProcessShaders([&](SHADER_TYPE shaderType, const ShaderResourceLayout& layout)
		{
				// TODO
		});
	}

	ShaderResourceBinding::~ShaderResourceBinding()
	{
	}

}