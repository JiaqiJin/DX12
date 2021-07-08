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

		// Record every resource used by Shader
		UINT skipCount = 1;
		for (UINT i = 0; i < DXshaderDesc.BoundResources; i += skipCount)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindingDesc = {};
			pShaderReflection->GetResourceBindingDesc(i, &bindingDesc);

			std::string name = bindingDesc.Name;

			UINT bindCount = bindingDesc.BindCount;

			// Process the array
			// In Shader Model 5_0 and previous versions, each array resource is listed separately.
			// For example, the following texture array is defined in Shader:
			//
			// Texture2D<float3> g_tex2DDiffuse[4];
			//
			// The Shader reflection system will enumerate four resources with the following names:
			// "g_tex2DDiffuse[0]"
			// "g_tex2DDiffuse[1]"
			// "g_tex2DDiffuse[2]"
			// "g_tex2DDiffuse[3]"
			//
			// If some of the elements of the array resource are not used by the Shader, they will not be listed
			auto openBracketPos = name.find('[');
			if (-1 != openBracketPos)
			{
				assert((bindCount == 1) && "When array elements are enumerated individually, BindCount is expected to always be 1");

				// Name == "g_tex2DDiffuse[0]"
				//                        ^
				//                   OpenBracketPos
				// Name without parentheses
				name.erase(openBracketPos, name.length() - openBracketPos);
				// Name == "g_tex2DDiffuse"

				for (UINT j = i + 1; j < DXshaderDesc.BoundResources; ++j)
				{
					D3D12_SHADER_INPUT_BIND_DESC arrayElementBindingDesc = {};
					pShaderReflection->GetResourceBindingDesc(j, &arrayElementBindingDesc);

					// strncmp：Equal returns 0
					if (strncmp(name.c_str(), arrayElementBindingDesc.Name, openBracketPos) == 0 && arrayElementBindingDesc.Name[openBracketPos] == '[')
					{
						// Convert a string to an int, the number of the string can contain other characters without affecting the result,
						// this function will not throw an exception
						UINT index = atoi(arrayElementBindingDesc.Name + openBracketPos + 1);
						bindCount = std::max(bindCount, index + 1);

						++skipCount;
					}
					// end
					else
					{
						break;
					}
				}
			}
			std::unique_ptr<ShaderResourceAttribs> shaderResourceAttribs = std::make_unique<ShaderResourceAttribs>(name, bindingDesc.BindPoint,
				bindCount, bindingDesc.Type, bindingDesc.Dimension);
			// SIT: Shader Input Type
			switch (bindingDesc.Type)
			{
			case D3D_SIT_CBUFFER:
				m_CBs.push_back(std::move(shaderResourceAttribs));
				break;
			case D3D_SIT_TEXTURE:
				if (bindingDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					m_BufferSRVs.push_back(std::move(shaderResourceAttribs));
				}
				else
				{
					m_TextureSRVs.push_back(std::move(shaderResourceAttribs));
				}
				break;
			case D3D_SIT_UAV_RWTYPED:
				if (bindingDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					m_BufferUAVs.push_back(std::move(shaderResourceAttribs));
				}
				else
				{
					m_TextureUAVs.push_back(std::move(shaderResourceAttribs));
				}
				break;
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
				m_BufferSRVs.push_back(std::move(shaderResourceAttribs));
				break;
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
				m_BufferUAVs.push_back(std::move(shaderResourceAttribs));
				break;
			default:
				LOG_ERROR("Not Supported Resource Type.");
				break;
			}
		}

		// TODO TEXTURE SAMPLER 
	}

	size_t ShaderResource::GetHash() const
	{
		size_t hash = ComputeHash(GetCBNum(), GetTexSRVNum(), GetTexUAVNum(), GetBufSRVNum(), GetBufUAVNum());

		for (UINT32 i = 0; i < m_CBs.size(); ++i)
		{
			const auto& cb = GetCB(i);
			HashCombine(hash, cb);
		}

		for (UINT32 i = 0; i < m_TextureSRVs.size(); ++i)
		{
			const auto& texSRV = GetTexSRV(i);
			HashCombine(hash, texSRV);
		}

		for (UINT32 i = 0; i < m_TextureUAVs.size(); ++i)
		{
			const auto& texUAV = GetTexUAV(i);
			HashCombine(hash, texUAV);
		}

		for (UINT32 i = 0; i < m_BufferSRVs.size(); ++i)
		{
			const auto& bufSRV = GetBufSRV(i);
			HashCombine(hash, bufSRV);
		}

		for (UINT32 i = 0; i < m_BufferUAVs.size(); ++i)
		{
			const auto& bufUAV = GetBufUAV(i);
			HashCombine(hash, bufUAV);
		}

		return hash;
	}

	bool ShaderResource::IsCompatibleWith(const ShaderResource& shaderResource) const
	{
		if(GetCBNum() != shaderResource.GetCBNum() ||
			GetTexSRVNum() != shaderResource.GetTexSRVNum() ||
			GetTexUAVNum() != shaderResource.GetTexUAVNum() ||
			GetBufSRVNum() != shaderResource.GetBufSRVNum() ||
			GetBufUAVNum() != shaderResource.GetBufUAVNum())
			return false;

		bool isCompatible = true;

		ProcessResources(
			[&](const ShaderResourceAttribs& cb, UINT32 i)
			{
				if (!cb.IsCompatibleWith(shaderResource.GetCB(i)))
					isCompatible = false;
			},
			[&](const ShaderResourceAttribs& texSRV, UINT32 i)
			{
				if (!texSRV.IsCompatibleWith(shaderResource.GetTexSRV(i)))
					isCompatible = false;
			},
				[&](const ShaderResourceAttribs& texUAV, UINT32 i)
			{
				if (!texUAV.IsCompatibleWith(shaderResource.GetTexUAV(i)))
					isCompatible = false;
			},
				[&](const ShaderResourceAttribs& bufSRV, UINT32 i)
			{
				if (!bufSRV.IsCompatibleWith(shaderResource.GetBufSRV(i)))
					isCompatible = false;
			},
				[&](const ShaderResourceAttribs& bufUAV, UINT32 i)
			{
				if (!bufUAV.IsCompatibleWith(shaderResource.GetBufUAV(i)))
					isCompatible = false;
			}
			);

		return isCompatible;
	}

	SHADER_RESOURCE_VARIABLE_TYPE ShaderResource::FindVariableType(const ShaderResourceAttribs& ResourceAttribs,
		const ShaderVariableConfig& shaderVariableConfig) const
	{
		return GetShaderVariableType(m_ShaderType, ResourceAttribs.Name, shaderVariableConfig);
	}
}