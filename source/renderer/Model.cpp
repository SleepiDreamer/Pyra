#include "Model.h"
#include "GPUAllocator.h"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <stdexcept>

using namespace DirectX;

Model::Model(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator, const std::filesystem::path& path)
    : m_name(path.stem().string())
{
    LoadGLTF(device, commandQueue, allocator, path);
}

Model::~Model() = default;

void Model::LoadGLTF(ID3D12Device10* device, CommandQueue& commandQueue, GPUAllocator& allocator, const std::filesystem::path& path)
{
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
    {
	    throw std::runtime_error("Failed to load glTF file: " + path.string());
    }

    constexpr auto options =
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::LoadExternalImages;

    auto asset = parser.loadGltf(data.get(), path.parent_path(), options);
    if (asset.error() != fastgltf::Error::None)
    {
	    throw std::runtime_error("Failed to parse glTF: " + path.string());
    }

    size_t sceneIndex = asset->defaultScene.value_or(0);
    const auto& scene = asset->scenes[sceneIndex];

    for (size_t nodeIndex : scene.nodeIndices)
    {
        TraverseNode(device, commandQueue, allocator, asset.get(), nodeIndex, XMMatrixIdentity());
    }
}

void Model::TraverseNode(ID3D12Device10* device, 
						 CommandQueue& commandQueue,
						 GPUAllocator& allocator,
					     const fastgltf::Asset& asset,
					     size_t nodeIndex,
					     const XMMATRIX& parentTransform)
{
    const auto& node = asset.nodes[nodeIndex];

    XMMATRIX localTransform = GetNodeTransform(node);
    XMMATRIX worldTransform = XMMatrixMultiply(localTransform, parentTransform);

    // Node has mesh
    if (node.meshIndex.has_value())
    {
        const auto& mesh = asset.meshes[node.meshIndex.value()];
        LoadMesh(device, commandQueue, allocator, asset, mesh, worldTransform);
    }

    // TODO: Handle lights when node.lightIndex.has_value()

    for (size_t childIndex : node.children)
    {
        TraverseNode(device, commandQueue, allocator, asset, childIndex, worldTransform);
    }
}

XMMATRIX Model::GetNodeTransform(const fastgltf::Node& node)
{
    return std::visit(fastgltf::visitor{
        [](const fastgltf::TRS& trs) -> XMMATRIX
        {
            XMVECTOR translation = XMVectorSet(trs.translation[0], trs.translation[1], trs.translation[2], 0.0f);
            XMVECTOR rotation = XMVectorSet(trs.rotation[0], trs.rotation[1], trs.rotation[2], trs.rotation[3]);
            XMVECTOR scale = XMVectorSet(trs.scale[0], trs.scale[1], trs.scale[2], 0.0f);

            return XMMatrixScalingFromVector(scale) *
                   XMMatrixRotationQuaternion(rotation) *
                   XMMatrixTranslationFromVector(translation);
        },
        [](const fastgltf::math::fmat4x4& matrix) -> XMMATRIX
        {
			// Transpose
            return XMMATRIX(
                matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0],
                matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1],
                matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2],
                matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]
            );
        }
        }, node.transform);
}

void Model::LoadMesh(ID3D12Device10* device, 
					 CommandQueue& commandQueue,
					 GPUAllocator& allocator,
				     const fastgltf::Asset& asset,
				     const fastgltf::Mesh& gltfMesh,
				     const XMMATRIX& transform)
{
    for (const auto& primitive : gltfMesh.primitives)
    {
        if (primitive.type != fastgltf::PrimitiveType::Triangles)
            continue; // Skip non-triangle primitives for now

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // Get position accessor (required)
        auto posIt = primitive.findAttribute("POSITION");
        if (posIt == primitive.attributes.end())
            continue;

        const auto& posAccessor = asset.accessors[posIt->accessorIndex];
        vertices.resize(posAccessor.count);

        // Load positions
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, posAccessor,
            [&](const fastgltf::math::fvec3& pos, size_t idx)
            {
                vertices[idx].position = XMFLOAT3(pos.x(), pos.y(), pos.z());
            });

        // Load normals
        auto normIt = primitive.findAttribute("NORMAL");
        if (normIt != primitive.attributes.end())
        {
            const auto& normAccessor = asset.accessors[normIt->accessorIndex];
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normAccessor,
                [&](const fastgltf::math::fvec3& norm, size_t idx)
                {
                    vertices[idx].normal = XMFLOAT3(norm.x(), norm.y(), norm.z());
                });
        }

        // Load texture coordinates
        auto texIt = primitive.findAttribute("TEXCOORD_0");
        if (texIt != primitive.attributes.end())
        {
            const auto& texAccessor = asset.accessors[texIt->accessorIndex];
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, texAccessor,
                [&](const fastgltf::math::fvec2& uv, size_t idx)
                {
                    vertices[idx].texCoord = XMFLOAT2(uv.x(), uv.y());
                });
        }

        // Load indices
        if (primitive.indicesAccessor.has_value())
        {
            const auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
            indices.resize(indexAccessor.count);

            fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indexAccessor,
                [&](uint32_t index, size_t idx)
                {
                    indices[idx] = index;
                });
        }
        else
        {
            indices.resize(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i)
                indices[i] = static_cast<uint32_t>(i);
        }

        Mesh mesh;
        mesh.materialIndex = primitive.materialIndex.has_value()
            ? static_cast<int32_t>(primitive.materialIndex.value()) : -1;

        XMStoreFloat4x4(&mesh.transform, transform);

        std::string meshName = m_name + "_" + std::string(gltfMesh.name) +
            "_prim" + std::to_string(m_meshes.size());

        mesh.Upload(allocator, vertices, indices, meshName);
        mesh.BuildBLAS(device, allocator, commandQueue);
        m_meshes.push_back(std::move(mesh));
    }
}