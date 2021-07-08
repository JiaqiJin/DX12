#pragma once

#include "../DescriptorHeap.h"
#include "../GpuBuffer.h"
#include "../GpuResourceDescriptor.h"

namespace RHI
{
    class RootSignature;
    class RenderDevice;
    class CommandContext;

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

        void Initialize(RenderDevice* device,
            const RootSignature* rootSignature,
            const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
            UINT32 allowedTypeNum);

        static constexpr UINT32 InvalidDescriptorOffset = static_cast<UINT32>(-1);

        // Currently only Constant Buffer is bound as Root Descriptor
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
            // Offset from the start of the descriptor heap allocation to the start of the table
            UINT32 TableStartOffset = InvalidDescriptorOffset;

            // The number of resources in the Table
            std::vector<std::shared_ptr<GpuResourceDescriptor>> Descriptors;
        };

        // ShaderResourceLayout obtains the "Descriptor Handle" through this function, and copies the Descriptor of the resource to be bound
        // OffsetFromTableStart is the offset in the Table, which is different from RootParam.m_TableStartOffset
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderVisibleTableCPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };

            if (RootParam.TableStartOffset != InvalidDescriptorOffset)
                CPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetCpuHandle(RootParam.TableStartOffset + OffsetFromTableStart);

            return CPUDescriptorHandle;
        }

        // RootSignature accesses the GPU Descriptor Handle through this function, and then submits it to the rendering pipeline
        // OffsetFromTableStart is the offset in the Table, which is different from RootParam.m_TableStartOffset
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            assert(RootParam.TableStartOffset != InvalidDescriptorOffset && "GPU descriptor handle must never be requested for dynamic resources");

            D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetGpuHandle(RootParam.TableStartOffset + OffsetFromTableStart);

            return GPUDescriptorHandle;
        }

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

        // Commite Static, Mutable binding resource
        void CommitResource(CommandContext& cmdContext);

        /* Submit the resource binding of Dynamic Shader Variable and the resource of Dynamic Resource, automatically submit before each Draw
         * Dynamic Resource is not equal to Dynamic Shader Variable, Dynamic Resource represents the frequency
         * at which the resource itself is modified, and Dynamic Shader Variable represents the frequency at which resource binding is switched
         * For example, the Shader Variable type of Transform's Constant Buffer is Static, but this Buffer is Dynamic Buffer
         */
        void CommitDynamic(CommandContext& cmdContext);

    private:
        // Allocation in a GPU-visible CBV/SRV/UAV descriptor heap
        DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;

        UINT32 m_NumDynamicDescriptor = 0;

        std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootDescriptors;
        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;

        ID3D12Device* m_D3D12Device;
    };
}