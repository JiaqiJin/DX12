#include "../../pch.h"
#include "ShaderResourceCache.h"
#include "../RootSignature.h"
#include "../RenderDevice.h"
#include "ShaderResourceBindingUtility.h"
#include "../CommandContext.h"

namespace RHI
{
    void ShaderResourceCache::Initialize(RenderDevice* device,
        const RootSignature* rootSignature,
        const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
        UINT32 allowedTypeNum)
    {
        assert(rootSignature != nullptr);

        const UINT32 allowedTypeBits = GetAllowedTypeBits(allowedVarTypes, allowedTypeNum);

        rootSignature->ProcessRootDescriptors([&](const RootParameter& rootDescriptor)
        {
            SHADER_RESOURCE_VARIABLE_TYPE variableType = rootDescriptor.GetShaderVariableType();
            UINT32 rootIndex = rootDescriptor.GetRootIndex();

            if (IsAllowedType(variableType, allowedTypeBits))
                m_RootDescriptors.insert(std::make_pair(rootIndex, RootDescriptor(variableType)));
        });

        UINT32 descriptorNum = 0;

        rootSignature->ProcessRootTables([&](const RootParameter& rootTable)
        {
            SHADER_RESOURCE_VARIABLE_TYPE variableType = rootTable.GetShaderVariableType();
            UINT32 rootIndex = rootTable.GetRootIndex();
            UINT32 rootTableSize = rootTable.GetDescriptorTableSize();
            const auto& D3D12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(rootTable);

            assert(D3D12RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
            assert(rootTableSize > 0 && "Unexpected empty descriptor table");

            if (IsAllowedType(variableType, allowedTypeBits))
            {
                m_RootTables.insert({ rootIndex, RootTable(variableType, rootTableSize) });

                if (variableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
                {
                    m_RootTables[rootIndex].TableStartOffset = descriptorNum;
                    descriptorNum += rootTableSize;
                }

                if (variableType == SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
                    m_NumDynamicDescriptor += rootTableSize;
            }
        });

        // Allocate space on the GPU Descriptor Heap
        if (descriptorNum)
        {
            m_CbvSrvUavGPUHeapSpace = device->AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorNum);
            assert(!m_CbvSrvUavGPUHeapSpace.IsNull() && "Failed to allocate  GPU-visible CBV/SRV/UAV descriptor");
        }

        m_D3D12Device = device->GetD3D12Device();
    }

    void ShaderResourceCache::CommitResource(CommandContext& cmdContext)
    {
        // Submit Root View (CBV), only need to bind the address of Buffer
        for (const auto& [rootIndex, rootDescriptor] : m_RootDescriptors)
        {
            // Both Dynamic Buffer and Dynamic Variable are submitted in Commit Dynamic before Draw
            if (rootDescriptor.VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
            {
                if(rootDescriptor.ConstantBuffer == nullptr)
                    LOG_ERROR("No Resource Binding");

                GpuDynamicBuffer* dynamicBuffer = dynamic_cast<GpuDynamicBuffer*>(rootDescriptor.ConstantBuffer.get());

                if (dynamicBuffer == nullptr)
                    cmdContext.GetGraphicsContext().SetConstantBuffer(rootIndex, rootDescriptor.ConstantBuffer->GetGpuVirtualAddress());
            }
        }

        // The Descriptor of Static and Mutable resources has been copied to the Heap of ShaderResourceCache and submitted directly
        for (const auto& [rootIndex, rootTable] : m_RootTables)
        {
            for (INT32 i = 0; i < rootTable.Descriptors.size(); ++i)
            {
                if (rootTable.Descriptors[i] == nullptr)
                    LOG_ERROR("No Resource Binding");
            }

            if (rootTable.VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
                cmdContext.GetGraphicsContext().SetDescriptorTable(rootIndex, GetShaderVisibleTableGPUDescriptorHandle
                    <D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(rootIndex, 0));
        }
    }

    void ShaderResourceCache::CommitDynamic(CommandContext& cmdContext)
    {
        for (const auto& [rootIndex, rootDescriptor] : m_RootDescriptors)
        {
            if (rootDescriptor.ConstantBuffer == nullptr)
                LOG_ERROR("No Resource Binding");

            GpuDynamicBuffer* dynamicBuffer = dynamic_cast<GpuDynamicBuffer*>(rootDescriptor.ConstantBuffer.get());

            if (dynamicBuffer != nullptr)
                cmdContext.GetGraphicsContext().SetConstantBuffer(rootIndex, dynamicBuffer->GetGpuVirtualAddress());
        }

        if (m_NumDynamicDescriptor > 0)
        {
           // Assign dynamic Descriptor Allocation
            DescriptorHeapAllocation dynamicAllocation = cmdContext.AllocateDynamicGPUVisibleDescriptor(m_NumDynamicDescriptor);
            UINT32 dynamicTableOffset = 0;

            for (const auto& [rootIndex, rootTable] : m_RootTables)
            {
                if (rootTable.VariableType == SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
                {
                    for (INT32 i = 0; i < rootTable.Descriptors.size(); ++i)
                    {
                        if (rootTable.Descriptors[i] == nullptr)
                            LOG_ERROR("No Resource Binding");
                    }

                    // Bind first and then Copy, because you want to bind the starting position of this Table, 
                     // you have to use the dynamicTableOffset before Copy
                    cmdContext.GetGraphicsContext().SetDescriptorTable(rootIndex, dynamicAllocation.GetGpuHandle(dynamicTableOffset));

                    // Copy the CPU Descriptor of the resource to the GPU Descriptor Heap
                    for (UINT32 i = 0; i < rootTable.Descriptors.size(); i++)
                    {
                        m_D3D12Device->CopyDescriptorsSimple(1, dynamicAllocation.GetCpuHandle(dynamicTableOffset),
                            rootTable.Descriptors[i]->GetCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                        dynamicTableOffset++;
                    }
                }
            }
        }
    }

} // end namespace