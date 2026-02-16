#include "Mesh.h"
#include "CommandQueue.h"
#include "GPUAllocator.h"
#include "BLAS.h"

Mesh::Mesh() = default;

Mesh::~Mesh()
{
    if (m_vertexBuffer.allocation)
        m_vertexBuffer.allocation->Release();
    if (m_indexBuffer.allocation)
        m_indexBuffer.allocation->Release();
}

Mesh::Mesh(Mesh&& other) noexcept
    : materialIndex(other.materialIndex), transform(other.transform), m_vertexBuffer(other.m_vertexBuffer)
    , m_indexBuffer(other.m_indexBuffer), m_vertexCount(other.m_vertexCount), m_indexCount(other.m_indexCount)
{
    other.m_vertexBuffer = {};
    other.m_indexBuffer = {};
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        if (m_vertexBuffer.allocation)
            m_vertexBuffer.allocation->Release();
        if (m_indexBuffer.allocation)
            m_indexBuffer.allocation->Release();

        m_vertexBuffer = other.m_vertexBuffer;
        m_indexBuffer = other.m_indexBuffer;
        m_vertexCount = other.m_vertexCount;
        m_indexCount = other.m_indexCount;
        materialIndex = other.materialIndex;
        transform = other.transform;

        other.m_vertexBuffer = {};
        other.m_indexBuffer = {};
    }
    return *this;
}

void Mesh::Upload(const GPUAllocator& allocator, const std::vector<Vertex>& vertices,
				  const std::vector<uint32_t>& indices, const std::string& name)
{
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    m_indexCount = static_cast<uint32_t>(indices.size());

    const uint64_t verticesSize = vertices.size() * sizeof(Vertex);
    const uint64_t indicesSize = indices.size() * sizeof(uint32_t);

    m_vertexBuffer = allocator.CreateBuffer(
        verticesSize,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_FLAG_NONE,
        D3D12_HEAP_TYPE_UPLOAD, // TODO: Use DEFAULT + staging
        (name + "_VB").c_str()
    );

    m_indexBuffer = allocator.CreateBuffer(
        indicesSize,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_FLAG_NONE,
        D3D12_HEAP_TYPE_UPLOAD, // TODO: Use DEFAULT + staging
        (name + "_IB").c_str()
    );

    void* mapped = nullptr;
    m_vertexBuffer.resource->Map(0, nullptr, &mapped);
    memcpy(mapped, vertices.data(), verticesSize);
    m_vertexBuffer.resource->Unmap(0, nullptr);

    m_indexBuffer.resource->Map(0, nullptr, &mapped);
    memcpy(mapped, indices.data(), indicesSize);
    m_indexBuffer.resource->Unmap(0, nullptr);
}

D3D12_RAYTRACING_GEOMETRY_DESC Mesh::GetGeometryDesc() const
{
    D3D12_RAYTRACING_GEOMETRY_DESC desc = {};
    desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    desc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
    desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    desc.Triangles.VertexCount = m_vertexCount;
    desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    desc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
    desc.Triangles.IndexCount = m_indexCount;
    desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    return desc;
}

void Mesh::BuildBLAS(ID3D12Device10* device, GPUAllocator& allocator, CommandQueue& commandQueue)
{
    m_blas = std::make_unique<BLAS>(allocator, commandQueue);
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometries = { GetGeometryDesc() };
    m_blas->Build(device, geometries);
}

D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexBufferView() const
{
    return {
        .BufferLocation = m_vertexBuffer.resource->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<UINT>(m_vertexBuffer.size),
        .StrideInBytes = sizeof(Vertex)
    };
}

D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexBufferView() const
{
    return {
        .BufferLocation = m_indexBuffer.resource->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<UINT>(m_indexBuffer.size),
        .Format = DXGI_FORMAT_R32_UINT
    };
}