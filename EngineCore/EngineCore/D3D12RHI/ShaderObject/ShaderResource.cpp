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