#pragma once

namespace RHI
{
	class RenderDevice;

	struct ShaderDesc
	{
		SHADER_TYPE ShaderType;
	};

	struct ShaderMacro
	{
		const std::string Name;
		const std::string Definition;
	};

	struct ShaderModel
	{
		UINT8 Major;
		UINT8 Minor;
	};

	struct ShaderCreateInfo
	{
		std::wstring FilePath;
		std::string entryPoint;
		ShaderMacro* Macros = nullptr;
		ShaderDesc Desc;
		ShaderModel SM = { 5, 1 };
	};

	class Shader
	{
	public:
		Shader(const ShaderCreateInfo& shaderCI);

		ID3DBlob* GetShaderByteCode() { return m_ShaderByteCode.Get(); }

		SHADER_TYPE GetShaderType() const { return m_Desc.ShaderType; }

	private:
		ShaderDesc m_Desc;

		Microsoft::WRL::ComPtr<ID3DBlob> m_ShaderByteCode;
	};
}