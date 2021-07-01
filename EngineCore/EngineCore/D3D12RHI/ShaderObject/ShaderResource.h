#pragma once

namespace RHI
{
	struct ShaderDesc;

	// Describe all resource used by Shader. implemented using Shader Reflection system
	// 
	class ShaderResource
	{
	public:
		ShaderResource(ID3DBlob* pShaderBytecode, const ShaderDesc& shaderDesc);

		ShaderResource(const ShaderResource&) = delete;
		ShaderResource(ShaderResource&&) = delete;
		ShaderResource& operator = (const ShaderResource&) = delete;
		ShaderResource& operator = (ShaderResource&&) = delete;

	private:
		// Shader Type (Vertex, Pixel...)
		const SHADER_TYPE m_ShaderType;
		// Shader Version
		UINT32 m_ShaderVersion = 0;
		// Shader name
		std::string m_ShaderName;
	};
}