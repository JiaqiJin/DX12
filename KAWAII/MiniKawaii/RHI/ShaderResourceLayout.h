#pragma once

#include "ShaderResource.h"
#include "ShaderResourceCache.h"
#include "GpuBuffer.h"
#include "GpuResourceDescriptor.h"
// http://diligentgraphics.com/diligent-engine/architecture/d3d12/shader-resource-layout/
// https://github.com/DiligentGraphics/DiligentCore/blob/6605548066bbff052ea1c1f7e4fec14446fa7727/Graphics/GraphicsEngineD3D12/include/ShaderResourceLayoutD3D12.hpp
namespace RHI
{
	struct ShaderVariableConfig;
	class RootSignature;

	// Define mappings between shader resource and descriptor in descriptor table.
	// HLSL shader registers are first mapped to descriptor in descriptor table as defined by Root Signature.
	// Descriptor then reference actual resource in GPU memory
	class ShaderResourceLayout
	{
	public:
		// Traverse all resource of a Shader, grouped by RootSignature into different DescriptorTable 
		// ShaderResourceLayoutResource records the information of Shader resources, RootIndex and OffsetFromTableStart
		ShaderResourceLayout(ID3D12Device* pd3d12Device,
			PIPELINE_TYPE pipelineType,
			const ShaderVariableConfig& shaderVariableConfig,
			const ShaderResource* shaderResource,
			RootSignature* rootSignature);

		// Represents a resource in Shader, and contains two additional information, RootIndex and OffsetFromTable
		struct Resource
		{
		public:
			Resource(const Resource&) = delete;
			Resource(Resource&&) = delete;
			Resource& operator = (const Resource&) = delete;
			Resource& operator = (Resource&&) = delete;

			static constexpr UINT32 InvalidSamplerId = 0xFFFF;
			static constexpr const UINT32 MaxSamplerId = InvalidSamplerId - 1;
			static constexpr const UINT32 InvalidOffset = static_cast<UINT32>(-1);
		};

	private:
		ID3D12Device* m_D3D12Device;
		// All resources in the Shader are divided into three vectors according to the update frequency
		std::vector<std::unique_ptr<Resource>> m_SrvCbvUavs[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
	};
}