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