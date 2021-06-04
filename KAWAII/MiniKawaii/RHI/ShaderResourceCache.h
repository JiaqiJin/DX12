#pragma once

#include "DescriptorHeap.h"
#include "GpuBuffer.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	class RootSignature;
	class RenderDevice;
	class CommandContext;

	// Shader resource cache provides the storage to keep references to object bound by application to shader resource.
	// Only provide a storage space (GPUDescriptorHeap)
	// To store resource bound to the Shader, ShaderResourceCache will allocate space on GPU-Visible Descriptor heap to store the 
	// Descriptor of the resource
	// Root View does not allocate space in GPUDesciptorHeap. it is directly bound to RootSignature.
	// Static and Mutable Root Table will allocate space in GPUDescriptorHeap, and Dynamic will allocate space dynamically in each Draw Call
	class ShaderResourceCache
	{
		friend class CommandContext;
	public:
		ShaderResourceCache() = default;

		ShaderResourceCache(const ShaderResourceCache&) = delete;
		ShaderResourceCache(ShaderResourceCache&&) = delete;
		ShaderResourceCache& operator = (const ShaderResourceCache&) = delete;
		ShaderResourceCache& operator = (ShaderResourceCache&&) = delete;

		~ShaderResourceCache() = default;

		void Initialize(RenderDevice* device, const RootSignature* rootSignature, 
			const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, UINT32 allowedTypeNum);

		static constexpr UINT32 InvalidDescriptorOffset = static_cast<UINT32>(-1);

		struct RootDescriptor
		{
			RootDescriptor(SHADER_RESOURCE_VARIABLE_TYPE _VariableType) : VariableType(_VariableType) {}

			SHADER_RESOURCE_VARIABLE_TYPE VariableType;
			std::shared_ptr<GpuBuffer> ConstantBuffer = nullptr;
		};

		struct RootTable
		{
			RootTable(SHADER_RESOURCE_VARIABLE_TYPE _VariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC, UINT32 tableSize = 0) :
				Descriptors(tableSize, nullptr),
				VariableType(_VariableType)
			{
			}

			SHADER_RESOURCE_VARIABLE_TYPE VariableType;
			UINT32 TableStartOffset = InvalidDescriptorOffset;
			std::vector<std::shared_ptr<GpuResourceDescriptor>> Descriptors;
		};

	private:
		// GPU Descriptor Heap
		DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;
		UINT32 m_NumDynamicDescriptor = 0;

		std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootDescriptors;
		std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;

		ID3D12Device* m_D3D12Device;
	};
}