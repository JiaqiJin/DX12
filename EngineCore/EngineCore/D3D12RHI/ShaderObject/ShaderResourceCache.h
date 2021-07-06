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
        // Allocation in a GPU-visible CBV/SRV/UAV descriptor heap
        DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;

        UINT32 m_NumDynamicDescriptor = 0;

        std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootDescriptors;
        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;

        ID3D12Device* m_D3D12Device;
    };
}