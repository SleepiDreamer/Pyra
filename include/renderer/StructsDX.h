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
	Albedo,
	Emissive,
	Metallic,
	Roughness,
	NormalMap,
	Normal,
	GeoNormal,
	Tangent,
	Bitangent,
	TangentW,
};

enum TonemapOperator
{
	Linear = 0,
	Aces,
	Reinhard,
	AgX,
	GT7,
};

struct RenderSettings
{
	DebugMode debugMode = None;
	uint32_t bounces = 2;
	float exposure = 25.0f;
	float skyExposure = 1.0f;
	float lightExposure = 1.0f;
	int32_t hdriIndex = -1;
	uint32_t frame = 0;
	TonemapOperator tonemapper = AgX;
	BOOL whiteFurnace = false;
	BOOL upscaling = false;
};
IMGUI_REFLECT(RenderSettings, debugMode, bounces, exposure, skyExposure, lightExposure, whiteFurnace, upscaling, tonemapper)