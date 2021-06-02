#pragma once

namespace RHI 
{
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const struct ShaderVariableConfig& shaderVariableConfig);

    bool IsConsistentShaderType(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType);

    INT32 GetShaderTypePipelineIndex(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType);
}