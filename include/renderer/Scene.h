#pragma once
#include "GPUBuffer.h"

class Model;
class TLAS;
class CommandQueue;
class GPUAllocator;

class Scene
{
public:
	Scene(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator);
	~Scene() = default;

	void LoadModel(const std::string& path);
	[[nodiscard]] const std::vector<Model>& GetModels() const { return m_models; }
	[[nodiscard]] const TLAS& GetTLAS() const { return *m_tlas; }
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetTLASAddress() const;
private:
	std::vector<Model> m_models;
	std::unique_ptr<TLAS> m_tlas;

	ID3D12Device10* m_device;
	CommandQueue& m_commandQueue;
	GPUAllocator& m_allocator;
};

