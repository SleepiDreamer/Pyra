#include "Model.h"

#include "GPUAllocator.h"

Model::Model(GPUAllocator& allocator, CommandQueue& commandQueue, const std::string& name)
	: name(name)
{
	// triangle vertex data:
	struct Vertex
	{
		float position[3];
		float uv[2];
	};
	const Vertex vertices[] = {		
		{{ 0.0f, 0.5f, 0.0f },		{ 0.5f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f },	{ 1.0f, 1.0f } },
		{ { -0.5f, -0.5f, 0.0f },	{ 0.0f, 1.0f } }};
	const uint16_t indices[] = { 0, 1, 2 };

	m_vertexBuffer = allocator.CreateBuffer(
		sizeof(vertices), D3D12_RESOURCE_STATE_GENERIC_READ, 
		D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, (name + " Vertex Buffer").c_str());
	m_indexBuffer = allocator.CreateBuffer(
		sizeof(indices), D3D12_RESOURCE_STATE_GENERIC_READ, 
		D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, (name + " Index Buffer").c_str());

	// Upload vertex data
	void* mapped = nullptr;
	m_vertexBuffer.resource->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));
	m_vertexBuffer.resource->Unmap(0, nullptr);

	// Upload index data
	m_indexBuffer.resource->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, sizeof(indices));
	m_indexBuffer.resource->Unmap(0, nullptr);
}

Model::~Model() = default;
