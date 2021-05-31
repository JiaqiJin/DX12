#include "../pch.h"
#include "ShaderResourceLayout.h"

namespace RHI
{
	ShaderResourceLayout::ShaderResourceLayout(ID3D12Device* pd3d12Device,
		PIPELINE_TYPE pipelineType,
		const ShaderVariableConfig& shaderVariableConfig,
		const ShaderResource* shaderResource,
		RootSignature* rootSignature) :
		m_D3D12Device(pd3d12Device)
	{
		// Determine RootIndex and OffsetFromTable throught RootSignature for each resource in ShaderResource, and then store it
		auto AddResource = [&](const ShaderResourceAttribs& Attribs,
			BindingResourceType              ResType,
			SHADER_RESOURCE_VARIABLE_TYPE   VarType) //
		{
			assert(rootSignature != nullptr);

			UINT32 RootIndex = Resource::InvalidRootIndex;
			UINT32 Offset = Resource::InvalidOffset;

			D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeType = GetDescriptorRangeType(ResType);
			SHADER_TYPE shaderType = shaderResource->GetShaderType();

			// Add RootSignature in the order in ShaderResource, and assign RootIndex and Offset
			// TODO
		};

	}

	bool ShaderResourceLayout::Resource::IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const
	{
		// TODO
		return true;
	}

	void ShaderResourceLayout::Resource::BindResource(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		assert(buffer != nullptr);
		// TODO
		assert(ResourceType == BindingResourceType::CBV);


	}

	void ShaderResourceLayout::Resource::BindResource(std::shared_ptr<GpuResourceDescriptor> descriptor,
		UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		// TODO
	}
}