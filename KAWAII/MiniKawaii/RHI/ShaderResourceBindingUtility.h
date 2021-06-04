#pragma once

namespace RHI 
{
    bool IsAllowedType(SHADER_RESOURCE_VARIABLE_TYPE varType, UINT32 allowedTypeBits) noexcept;

    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const struct ShaderVariableConfig& shaderVariableConfig);

    UINT32 GetAllowedTypeBits(const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, UINT32 allowedTypeNum) noexcept;

    bool IsConsistentShaderType(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType);

    INT32 GetShaderTypePipelineIndex(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType);
}