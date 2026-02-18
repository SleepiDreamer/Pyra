#pragma once
#include <d3d12.h>
#include <glm/vec3.hpp>

struct CameraData
{
	glm::vec3 position;
	float fov;
	glm::vec3 forward;
	uint32_t _pad0;
	glm::vec3 right;
	uint32_t _pad1;
	glm::vec3 up;
	uint32_t _pad2;
};

struct HitGroupRecord
{
	D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS indexBuffer;
	uint32_t materialIndex;
	uint32_t padding;
};

struct MaterialData
{
	uint32_t albedoIndex;
	uint32_t metallicRoughnessIndex;
	uint32_t normalIndex;
	uint32_t emissionIndex;
};