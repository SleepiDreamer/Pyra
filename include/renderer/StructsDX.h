#pragma once
#include <glm/vec3.hpp>

struct CameraData
{
	glm::vec3 position;
	float fov;
	glm::vec3 forward;
	float _pad0;
	glm::vec3 right;
	float _pad1;
	glm::vec3 up;
	float _pad2;
};

struct MaterialData
{
	uint32_t albedoIndex;
	uint32_t metallicRoughnessIndex;
	uint32_t normalIndex;
	uint32_t emissionIndex;
};