#pragma once

namespace RHI 
{
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const struct ShaderVariableConfig& shaderVariableConfig);

    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType);
}