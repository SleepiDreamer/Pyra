#pragma once
#include "GPUBuffer.h"

class Model;
class TLAS;
class CommandQueue;
class GPUAllocator;

class Scene
{
public:
	Scene(RenderContext& context);
	~Scene() = default;

	void LoadModel(const std::string& path);

	[[nodiscard]] const std::vector<Model>& GetModels() const { return m_models; }
	[[nodiscard]] const TLAS& GetTLAS() const { return *m_tlas; }
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetTLASAddress() const;
	[[nodiscard]] std::vector<HitGroupRecord> GetHitGroupRecords() const;
private:
	RenderContext& m_context;
	std::vector<Model> m_models;
	std::unique_ptr<TLAS> m_tlas;
};

