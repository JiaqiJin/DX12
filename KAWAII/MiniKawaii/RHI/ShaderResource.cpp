#include "../pch.h"
#include "ShaderResource.h"
#include "Shader.h"

namespace RHI
{
	ShaderResource::ShaderResource(ID3DBlob* pShaderBytecode, const ShaderDesc& shaderDesc) 
		: m_ShaderType(shaderDesc.ShaderType)
	{

	}
}