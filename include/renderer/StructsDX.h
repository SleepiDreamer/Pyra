#pragma once
#include <d3d12.h>
#include <glm/vec3.hpp>
#include <ImReflect.hpp>

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
IMGUI_REFLECT(CameraData, position, fov, forward, right, up)

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
	glm::vec3 emissiveFactor = glm::vec3(1.0f);
	int32_t emissiveIndex = -1;
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;
	int32_t metallicRoughnessIndex = -1;
	int32_t normalIndex = -1;
	uint32_t _pad0;
	uint32_t _pad1;
};


enum DebugMode
{
	None = 0,
	Albedo = 1,
	Emissive = 2,
	Metallic = 3,
	Roughness = 4,
	Normal = 5
};

enum TonemapOperator
{
	Linear = 0,
	Aces = 1,
	Reinhard = 2,
	AgX = 3
};

struct RenderSettings
{
	DebugMode debugMode = None;
	uint32_t bounces = 2;
	float exposure = 1.0f;
	float skyExposure = 1.0f;
	float lightExposure = 1.0f;
	BOOL whiteFurnace = false;
	BOOL upscaling = false;
	uint32_t frame = 0;
	TonemapOperator tonemapper = AgX;
	uint32_t _pad0;
};
IMGUI_REFLECT(RenderSettings, debugMode, bounces, exposure, skyExposure, lightExposure, whiteFurnace, upscaling, tonemapper)