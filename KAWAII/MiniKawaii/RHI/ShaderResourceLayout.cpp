#include "../pch.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceBindingUtility.h"
#include "RootSignature.h"
#include "RenderDevice.h"

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
			rootSignature->AllocateResourceSlot(shaderType, pipelineType, Attribs, VarType, DescriptorRangeType, RootIndex, Offset);

			std::unique_ptr<Resource> resource = std::make_unique<Resource>(Attribs, VarType, ResType, RootIndex, Offset);
			m_SrvCbvUavs[VarType].push_back(std::move(resource));
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

	bool ShaderResourceLayout::Resource::IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const
	{
		if (ResourceType == BindingResourceType::CBV)
		{
			const ShaderResourceCache::RootDescriptor& rootDescriptor = resourceCache.GetRootDescriptor(RootIndex);
			if (rootDescriptor.ConstantBuffer != nullptr)
				return true;
		}
		else
		{
			const ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);
			if (OffsetFromTableStart + arrayIndex < rootTable.Descriptors.size())
			{
				if (rootTable.Descriptors[OffsetFromTableStart + arrayIndex] != nullptr)
					return true;
			}
		}

		return false;
	}

	void ShaderResourceLayout::Resource::BindResource(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		assert(buffer != nullptr);
		assert(ResourceType == BindingResourceType::CBV);

		ShaderResourceCache::RootDescriptor& rootDescriptor = resourceCache.GetRootDescriptor(RootIndex);

		if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && rootDescriptor.ConstantBuffer != nullptr)
		{

			return;
		}

		rootDescriptor.ConstantBuffer = buffer;
	}

	void ShaderResourceLayout::Resource::BindResource(std::shared_ptr<GpuResourceDescriptor> descriptor,
		UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		assert(descriptor != nullptr);

		ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);
		// If the resource is already bound, it will not be updated, unless it is a Dynamic resource
		if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && rootTable.Descriptors[OffsetFromTableStart + arrayIndex] != nullptr)
		{
			return;
		}
		// Save reference to the descriptor object
		rootTable.Descriptors[arrayIndex + OffsetFromTableStart] = descriptor;

		D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle =
			resourceCache.GetShaderVisibleTableCPUDescriptorHandle<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(RootIndex, OffsetFromTableStart + arrayIndex);

		if (shaderVisibleHeapCPUDescriptorHandle.ptr != 0)
		{
			ID3D12Device* d3d12Device = RenderDevice::GetSingleton().GetD3D12Device();
			d3d12Device->CopyDescriptorsSimple(1,
				shaderVisibleHeapCPUDescriptorHandle,
				descriptor->GetCpuHandle(),
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}
}