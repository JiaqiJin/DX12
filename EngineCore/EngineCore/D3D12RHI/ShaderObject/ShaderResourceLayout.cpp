#include "../../pch.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceBindingUtility.h"
#include "../RootSignature.h"
#include "../RenderDevice.h"

namespace RHI
{
	ShaderResourceLayout::ShaderResourceLayout(ID3D12Device* pd3d12Device,
		PIPELINE_TYPE pipelineType,
		const ShaderVariableConfig& shaderVariableConfig,
		const ShaderResource* shaderResource,
		RootSignature* rootSignature) :
		m_D3D12Device(pd3d12Device)
	{
		// Determine RootIndex and OffsetFromTableStart through RootSignature for each resource in ShaderResource, and then store it
		auto AddResource = [&](const ShaderResourceAttribs& Attribs, BindingResourceType ResType, SHADER_RESOURCE_VARIABLE_TYPE VarType)
		{
			assert(rootSignature != nullptr);

			// TODO
		};

		shaderResource->ProcessResources(
			[&](const ShaderResourceAttribs& CB, UINT32)
			{
				auto VarType = shaderResource->FindVariableType(CB, shaderVariableConfig);
				AddResource(CB, BindingResourceType::CBV, VarType);
			},
			[&](const ShaderResourceAttribs& TexSRV, UINT32)
			{
				auto VarType = shaderResource->FindVariableType(TexSRV, shaderVariableConfig);
				AddResource(TexSRV, BindingResourceType::TexSRV, VarType);
			},
				[&](const ShaderResourceAttribs& TexUAV, UINT32)
			{
				auto VarType = shaderResource->FindVariableType(TexUAV, shaderVariableConfig);
				AddResource(TexUAV, BindingResourceType::TexUAV, VarType);
			},
				[&](const ShaderResourceAttribs& BufSRV, UINT32)
			{
				auto VarType = shaderResource->FindVariableType(BufSRV, shaderVariableConfig);
				AddResource(BufSRV, BindingResourceType::BufSRV, VarType);
			},
				[&](const ShaderResourceAttribs& BufUAV, UINT32)
			{
				auto VarType = shaderResource->FindVariableType(BufUAV, shaderVariableConfig);
				AddResource(BufUAV, BindingResourceType::BufUAV, VarType);
			}
			);
	}
}