#pragma once

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

struct ViewSettings
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	float distance;
	float fov;
};

struct SceneSettings
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	
	static const int NumLights = 3;
	struct Light {
		glm::vec3 direction;
		glm::vec3 radiance;
		bool enabled = false;
	} lights[NumLights];
};