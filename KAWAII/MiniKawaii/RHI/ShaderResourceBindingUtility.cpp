#include "../pch.h"
#include "ShaderResourceBindingUtility.h"
#include "PipelineState.h"

namespace RHI
{
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const ShaderVariableConfig& shaderVariableConfig)
    {
        for (UINT32 i = 0; i < shaderVariableConfig.Variables.size(); ++i)
        {
            const auto& curVarDesc = shaderVariableConfig.Variables[i];

            if (((curVarDesc.ShaderType & shaderType) != 0) && (name == curVarDesc.name))
            {
                return curVarDesc.Type;
            }
        }
    }

    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType)
    {
        switch (cachedResType)
        {
        case BindingResourceType::CBV:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case BindingResourceType::BufSRV:
        case BindingResourceType::TexSRV:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        case BindingResourceType::BufUAV:
        case BindingResourceType::TexUAV:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        case BindingResourceType::Sampler:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        default:
            LOG_ERROR("Unkown CachedResourceType.");
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }
}