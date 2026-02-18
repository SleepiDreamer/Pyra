#pragma once
#include "GPUBuffer.h"
#include "DescriptorHeap.h"

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
	void UploadMaterials(RenderContext& context);

	[[nodiscard]] const std::vector<Model>& GetModels() const { return m_models; }
	[[nodiscard]] const TLAS& GetTLAS() const { return *m_tlas; }
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetTLASAddress() const;
	[[nodiscard]] std::vector<HitGroupRecord> GetHitGroupRecords() const;
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetMaterialsBufferAddress() const { return m_materialData.resource->GetGPUVirtualAddress(); }
private:
	RenderContext& m_context;
	std::unique_ptr<TLAS> m_tlas;
	std::vector<Model> m_models;
	GPUBuffer m_materialData;
	DescriptorHeap::Allocation m_materialSRV;
};

