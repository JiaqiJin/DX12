#include "../pch.h"
#include "Shader.h"

namespace RHI
{
	// Shader Profile
	std::string GetHLSLProfileString(SHADER_TYPE shaderType, ShaderModel sm)
	{
        std::string strShaderProfile;
        switch (shaderType)
        {
        case SHADER_TYPE_VERTEX:        strShaderProfile = "vs"; break;
        case SHADER_TYPE_PIXEL:         strShaderProfile = "ps"; break;
        case SHADER_TYPE_GEOMETRY:      strShaderProfile = "gs"; break;
        case SHADER_TYPE_HULL:          strShaderProfile = "hs"; break;
        case SHADER_TYPE_DOMAIN:        strShaderProfile = "ds"; break;
        case SHADER_TYPE_COMPUTE:       strShaderProfile = "cs"; break;
        case SHADER_TYPE_AMPLIFICATION: strShaderProfile = "as"; break;
        case SHADER_TYPE_MESH:          strShaderProfile = "ms"; break;
        default: LOG_ERROR("Unknown shader type");
        }

        strShaderProfile += "_";
        strShaderProfile += std::to_string(sm.Major);
        strShaderProfile += "_";
        strShaderProfile += std::to_string(sm.Minor);

        return strShaderProfile;
	}

	Shader::Shader(const ShaderCreateInfo& shaderCI)
		: m_Desc(shaderCI.Desc)
	{
        std::string strShaderProfile = GetHLSLProfileString(shaderCI.Desc.ShaderType, shaderCI.SM);

		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		HRESULT hr = S_OK;

        hr = D3DCompileFromFile(shaderCI.FilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            shaderCI.entryPoint.c_str(), strShaderProfile.c_str(), compileFlags, 0, &m_ShaderByteCode, &errors);

        if (errors != nullptr)
            LOG_ERROR((char*)errors->GetBufferPointer());

        ThrowIfFailed(hr);

        m_ShaderResource = std::make_unique<const ShaderResource>(m_ShaderByteCode.Get(), shaderCI.Desc);
	}
}