#include "../../pch.h"
#include "ShaderResource.h"
#include "../Shader.h"

namespace RHI
{
	ShaderResource::ShaderResource(ID3DBlob* pShaderBytecode, const ShaderDesc& shaderDesc) :
		m_ShaderType{ shaderDesc.ShaderType }
	{
		// Use reflection to get the resources that this Shader needs to bind
		ID3D12ShaderReflection* pShaderReflection;
		ThrowIfFailed(D3DReflect(pShaderBytecode->GetBufferPointer(),
			pShaderBytecode->GetBufferSize(),
			__uuidof(pShaderReflection), reinterpret_cast<void**>(&pShaderReflection)));

		D3D12_SHADER_DESC DXshaderDesc = {};
		pShaderReflection->GetDesc(&DXshaderDesc);

		m_ShaderVersion = DXshaderDesc.Version;

		// TODO

	}
}