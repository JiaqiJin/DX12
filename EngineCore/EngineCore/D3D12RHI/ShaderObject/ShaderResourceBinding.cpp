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
				std::unique_ptr<ShaderVariableCollection> variables = std::make_unique<ShaderVariableCollection>(&m_ShaderResourceCache,
					layout,
					allowedVarTypes,
					allowedTypeNum);
				m_ShaderVariableManagers.emplace(shaderType, std::move(variables));
		});
	}

	ShaderResourceBinding::~ShaderResourceBinding()
	{
	}

	ShaderVariable* ShaderResourceBinding::GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name)
	{
		auto iter = m_ShaderVariableManagers.find(ShaderType);
		if (iter == m_ShaderVariableManagers.end())
			return nullptr;

		return iter->second->GetVariable(Name);
	}

	UINT32 ShaderResourceBinding::GetVariableCount(SHADER_TYPE ShaderType) const
	{
		auto iter = m_ShaderVariableManagers.find(ShaderType);
		if (iter == m_ShaderVariableManagers.end())
			return 0;

		return iter->second->GetVariableCount();
	}

	ShaderVariable* ShaderResourceBinding::GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index)
	{
		auto iter = m_ShaderVariableManagers.find(ShaderType);
		if (iter == m_ShaderVariableManagers.end())
			return nullptr;

		return iter->second->GetVariable(Index);
	}
}