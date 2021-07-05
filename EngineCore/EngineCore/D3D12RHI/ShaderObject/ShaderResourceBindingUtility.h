#pragma once

namespace RHI
{
    bool IsAllowedType(SHADER_RESOURCE_VARIABLE_TYPE varType, UINT32 allowedTypeBits) noexcept;

    UINT32 GetAllowedTypeBits(const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, UINT32 allowedTypeNum) noexcept;

    // Determine whether Shader Type and Pipeline Type are compatible. For example, Compute Pipeline cannot have VS and PS
    bool IsConsistentShaderType(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    // Use Shader Type as an index, which will be used when constructing Root Table
    INT32 GetShaderTypePipelineIndex(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    // Find the Variable Type (Static, Mutable, Dynamic) of a ShaderResource from ShaderVariableConfig
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const struct ShaderVariableConfig& shaderVariableConfig);

    // CachedResourceType to D3D12_DESCRIPTOR_RANGE_TYPE
    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType);

    D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType);
}
