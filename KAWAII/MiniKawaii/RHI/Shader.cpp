#include "../pch.h"
#include "Shader.h"

namespace RHI
{
	Shader::Shader(const ShaderCreateInfo& shaderCI)
		: m_Desc(shaderCI.Desc)
	{
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		HRESULT hr = S_OK;

		/*hr = D3DCompileFromFile(shaderCI.FilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
			shaderCI.entryPoint.c_str(), )*/
	}
}