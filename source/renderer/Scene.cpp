#include "Scene.h"
#include "Model.h"
#include "TLAS.h"
#include "CommandQueue.h"
#include "GPUAllocator.h"


Scene::Scene(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator)
	: m_device(device), m_commandQueue(commandQueue), m_allocator(allocator)
{
}

void Scene::LoadModel(const std::string& path)
{
	m_models.emplace_back(m_device, m_commandQueue, m_allocator, path);
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances = {};
	for (const auto& model : m_models)
	{
		for (const auto& mesh : model.GetMeshes())
		{
			instances.emplace_back(mesh.GetInstanceDesc());
		}
	}
	m_tlas = std::make_unique<TLAS>(m_device, m_allocator, m_commandQueue);
	m_tlas->Build(m_device, instances);
}

D3D12_GPU_VIRTUAL_ADDRESS Scene::GetTLASAddress() const
{
	return m_tlas->GetResource().resource->GetGPUVirtualAddress();
}
