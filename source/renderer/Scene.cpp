#include "Scene.h"
#include "Model.h"
#include "TLAS.h"
#include "CommandQueue.h"
#include "GPUAllocator.h"


Scene::Scene(RenderContext& context)
	: m_context(context)
{
}

void Scene::LoadModel(const std::string& path)
{
	m_models.emplace_back(m_context, path);
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances = {};
	int instanceId = 0;
	for (const auto& model : m_models)
	{
		for (const auto& mesh : model.GetMeshes())
		{
			instances.emplace_back(mesh.GetInstanceDesc(instanceId++));
		}
	}
	m_tlas = std::make_unique<TLAS>(m_context);
	m_tlas->Build(m_context.device, instances);
}

D3D12_GPU_VIRTUAL_ADDRESS Scene::GetTLASAddress() const
{
	return m_tlas->GetResource().resource->GetGPUVirtualAddress();
}

std::vector<HitGroupRecord> Scene::GetHitGroupRecords() const
{
	std::vector<HitGroupRecord> records;
	for (const auto& model : m_models)
	{
		for (const auto& mesh : model.GetMeshes())
		{
			records.push_back({
				mesh.GetVertexBuffer()->GetGPUVirtualAddress(),
				mesh.GetIndexBuffer()->GetGPUVirtualAddress()
				});
		}
	}
	return records;
}