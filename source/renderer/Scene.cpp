#include "Scene.h"
#include "Model.h"
#include "Mesh.h"
#include "TLAS.h"
#include "CommandQueue.h"
#include "UploadContext.h"
#include "GPUAllocator.h"
#include "StructsDX.h"

Scene::Scene(RenderContext& context)
	: m_context(context)
{
}

void Scene::LoadModel(const std::string& path)
{
	auto commandList = m_context.commandQueue->GetCommandList();
	m_models.emplace_back(m_context, commandList.Get(), path);

	m_context.uploadContext->Flush();

	m_context.commandQueue->ExecuteCommandList(commandList);
	m_context.commandQueue->Flush();

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

	UploadMaterials();

	m_context.uploadContext->Flush();

	commandList = m_context.commandQueue->GetCommandList();

	for (auto& model : m_models)
	{
		for (auto& tex : model.GetTextures())
		{
			if (tex.GetResource())
			{
				TransitionResource(commandList.Get(), tex.GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
		}
	}

	m_context.commandQueue->ExecuteCommandList(commandList);
	m_context.commandQueue->Flush();
}

void Scene::UploadMaterials()
{
	std::vector<MaterialData> materials;
	for (const auto& model : m_models)
	{
		for (const auto& texture : model.GetMaterials())
		{
			materials.push_back({
				texture.albedoFactor,
				texture.albedoIndex,
				texture.emissiveFactor,
				texture.emissiveIndex,
				texture.metallicFactor,
				texture.roughnessFactor,
				texture.metallicRoughnessIndex,
				texture.normalIndex
			});
		}
	}

	uint64_t size = materials.size() * sizeof(MaterialData);
	m_materialData = m_context.allocator->CreateBuffer(
		size, D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_DEFAULT, "Materials");
	m_context.uploadContext->Upload(m_materialData, materials.data(), size);

	m_materialSRV = m_context.descriptorHeap->Allocate();
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.NumElements = static_cast<UINT>(materials.size());
	desc.Buffer.StructureByteStride = sizeof(MaterialData);
	desc.Format = DXGI_FORMAT_UNKNOWN;
	m_context.device->CreateShaderResourceView(m_materialData.resource, &desc, m_materialSRV.cpuHandle);
}

D3D12_GPU_VIRTUAL_ADDRESS Scene::GetTLASAddress() const
{
	return m_tlas->GetResource().resource->GetGPUVirtualAddress();
}

std::vector<HitGroupRecord> Scene::GetHitGroupRecords() const
{
	std::vector<HitGroupRecord> records;
	uint32_t materialOffset = 0;
	for (const auto& model : m_models)
	{
		for (const auto& mesh : model.GetMeshes())
		{
			HitGroupRecord rec{};
			rec.vertexBuffer = mesh.GetVertexBuffer()->GetGPUVirtualAddress();
			rec.indexBuffer = mesh.GetIndexBuffer()->GetGPUVirtualAddress();
			rec.materialIndex = mesh.m_materialIndex >= 0
				? materialOffset + static_cast<uint32_t>(mesh.m_materialIndex)
				: 0;
			records.push_back(rec);
		}
		materialOffset += static_cast<uint32_t>(model.GetMaterials().size());
	}
	return records;
}