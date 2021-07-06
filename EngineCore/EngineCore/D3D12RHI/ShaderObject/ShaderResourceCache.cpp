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
}