#pragma once

namespace RHI
{
	static constexpr UINT32 MAX_SHADERS_IN_PIPELINE = 5;

	enum SHADER_TYPE
	{
		SHADER_TYPE_UNKNOWN = 0x000,        ///< Unknown shader type
		SHADER_TYPE_VERTEX = 0x001,         ///< Vertex shader
		SHADER_TYPE_PIXEL = 0x002,          ///< Pixel (fragment) shader
		SHADER_TYPE_GEOMETRY = 0x004,       ///< Geometry shader
		SHADER_TYPE_HULL = 0x008,           ///< Hull (tessellation control) shader
		SHADER_TYPE_DOMAIN = 0x010,         ///< Domain (tessellation evaluation) shader
		SHADER_TYPE_COMPUTE = 0x020,        ///< Compute shader
		SHADER_TYPE_AMPLIFICATION = 0x040,  ///< Amplification (task) shader
		SHADER_TYPE_MESH = 0x080,           ///< Mesh shader
		SHADER_TYPE_LAST = SHADER_TYPE_MESH
	};


	enum PIPELINE_TYPE
	{
		PIPELINE_TYPE_GRAPHIC,
		PIPELINE_TYPE_COMPUTE,
		PIPELINE_TYPE_MESH
	};
}