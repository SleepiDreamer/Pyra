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
	uint32_t _pad0;
};

struct MaterialData
{
	glm::vec3 albedoFactor = glm::vec3(1.0f);
	int32_t albedoIndex = -1;
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;
	int32_t metallicRoughnessIndex = -1;
	int32_t normalIndex = -1;
	int32_t emissiveIndex = -1;
	int32_t _pad0 = -1;
};