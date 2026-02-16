#pragma once
#include "GPUBuffer.h"

#include <DirectXMath.h>
#include <string>

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
};

class CommandQueue;
class GPUAllocator;
class BLAS;

class Mesh
{
public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void Upload(const GPUAllocator& allocator,
		        const std::vector<Vertex>& vertices,
		        const std::vector<uint32_t>& indices,
		        const std::string& name);

    void BuildBLAS(ID3D12Device10* device, GPUAllocator& allocator, CommandQueue& commandQueue);

    [[nodiscard]] D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc() const;
    [[nodiscard]] ID3D12Resource* GetVertexBuffer() const { return m_vertexBuffer.resource; }
    [[nodiscard]] ID3D12Resource* GetIndexBuffer() const { return m_indexBuffer.resource; }
    [[nodiscard]] uint32_t GetVertexCount() const { return m_vertexCount; }
    [[nodiscard]] uint32_t GetIndexCount() const { return m_indexCount; }
    [[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    [[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

    int32_t materialIndex = -1;
    DirectX::XMFLOAT4X4 transform;

private:
    GPUBuffer m_vertexBuffer;
    GPUBuffer m_indexBuffer;
    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;
    std::unique_ptr<BLAS> m_blas = nullptr;
};