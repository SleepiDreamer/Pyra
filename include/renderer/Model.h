#pragma once
#include "Mesh.h"

#include <filesystem>
#include <fastgltf/core.hpp>

class GPUAllocator;

class Model
{
public:
    Model(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator,
          const std::filesystem::path& path);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    [[nodiscard]] const std::vector<Mesh>& GetMeshes() const { return m_meshes; }
    [[nodiscard]] const std::string& GetName() const { return m_name; }

private:
    void LoadGLTF(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator, const std::filesystem::path& path);

    void TraverseNode(ID3D12Device10* device, CommandQueue& commandQueue,
                      GPUAllocator& allocator, const fastgltf::Asset& asset, size_t nodeIndex, const DirectX::XMMATRIX& parentTransform);

    static DirectX::XMMATRIX GetNodeTransform(const fastgltf::Node& node);

    void LoadMesh(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator,
                  const fastgltf::Asset& asset, const fastgltf::Mesh& gltfMesh, const DirectX::XMMATRIX& transform);

    std::vector<Mesh> m_meshes;
    std::string m_name;
};