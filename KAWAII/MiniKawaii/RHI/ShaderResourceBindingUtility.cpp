#include "../pch.h"
#include "ShaderResourceBindingUtility.h"
#include "PipelineState.h"

namespace RHI
{
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType,
        const std::string& name,
        const ShaderVariableConfig& shaderVariableConfig)
    {
        SHADER_RESOURCE_VARIABLE_TYPE type = SHADER_RESOURCE_VARIABLE_TYPE::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
        // TODO
        return type;
    }
}