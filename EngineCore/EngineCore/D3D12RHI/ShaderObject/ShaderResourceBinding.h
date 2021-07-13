#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI
{
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

	private:
		PipelineState* m_PSO;

		ShaderResourceCache m_ShaderResourceCache;
		
	};
}