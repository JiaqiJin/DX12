#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI
{
	class PipelineState;

	// Manage resource binding
	class ShaderResourceBinding
	{
		friend class PipelineState;
	public:
		ShaderResourceBinding(PipelineState* PSO,
			const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
			UINT32 allowedTypeNum);

		~ShaderResourceBinding();

		ShaderResourceBinding(const ShaderResourceBinding& rhs) = delete;
		ShaderResourceBinding& operator=(const ShaderResourceBinding& rhs) = delete;
		ShaderResourceBinding(ShaderResourceBinding&& rhs) = delete;
		ShaderResourceBinding& operator=(const ShaderResourceBinding&& rhs) = delete;

		ShaderVariable* GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name);
		UINT32 GetVariableCount(SHADER_TYPE ShaderType) const;
		ShaderVariable* GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index);

	private:
		PipelineState* m_PSO;

		ShaderResourceCache m_ShaderResourceCache;
		// An SRB object can bind all Shader resources in a pipeline, so there are several ShaderVariableManagers if there are several Shaders, 
		// and one ShaderVariableManager represents a Shader variable
		std::unordered_map<SHADER_TYPE, std::unique_ptr<ShaderVariableCollection>> m_ShaderVariableManagers;
	};
}