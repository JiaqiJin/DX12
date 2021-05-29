#pragma once

namespace RHI 
{
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const struct ShaderVariableConfig& shaderVariableConfig);
}