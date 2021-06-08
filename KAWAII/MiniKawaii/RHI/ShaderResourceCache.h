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

		// ShaderResourceLayout obtains the Descriptor Handle through this function, and copies the Descriptor of the resource to be bound!!!
		// OffsetFromTableStart is the offset in the Table, which is different from RootParam.m_TableStartOffset
		template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
		D3D12_CPU_DESCRIPTOR_HANDLE GetShaderVisibleTableCPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0)
		{
			const auto& RootParam = GetRootTable(RootIndex);

			D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
			
			if (RootParam.TableStartOffset != InvalidDescriptorOffset)
			{
				CPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetCpuHandle(RootParam.TableStartOffset + OffsetFromTableStart);
			}

			return CPUDescriptorHandle;
		}

		// RootSignature uses this function to access the GPU Descriptor Handle, and then submits it to the rendering pipeline!!!
		// OffsetFromTableStart is the offset in the Table, which is different from RootParam.m_TableStartOffset
		template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
		D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
		{
			const auto& RootParam = GetRootTable(RootIndex);

			assert(RootParam.TableStartOffset != InvalidDescriptorOffset && "GPU descriptor handle must never be requested for dynamic resources");

			D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetGpuHandle(RootParam.TableStartOffset + OffsetFromTableStart);

			return GPUDescriptorHandle;
		}

		void CommitResource(CommandContext& cmdContext);

		// Getters
		RootDescriptor& GetRootDescriptor(UINT32 RootIndex)
		{
			return m_RootDescriptors.at(RootIndex);
		}

		const RootDescriptor& GetRootDescriptor(UINT32 RootIndex) const
		{
			return m_RootDescriptors.at(RootIndex);
		}

		RootTable& GetRootTable(UINT32 RootIndex)
		{
			return m_RootTables.at(RootIndex);
		}

		const RootTable& GetRootTable(UINT32 RootIndex) const
		{
			return m_RootTables.at(RootIndex);
		}

	private:
		// GPU Descriptor Heap
		DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;
		UINT32 m_NumDynamicDescriptor = 0;

		std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootDescriptors;
		std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;

		ID3D12Device* m_D3D12Device;
	};
}