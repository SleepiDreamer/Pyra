#include "TLAS.h"
#include "GPUAllocator.h"
#include "CommandQueue.h"

TLAS::TLAS(ID3D12Device5* device, GPUAllocator& allocator, CommandQueue& commandQueue)
	: m_allocator(allocator), m_commandQueue(commandQueue)
{
}

TLAS::~TLAS() = default;

void TLAS::Build(ID3D12Device5* device, const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances)
{
    uint64_t instancesSize = instances.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
    m_tlasInstances = m_allocator.CreateBuffer(
        instancesSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE, 
        D3D12_HEAP_TYPE_UPLOAD, "TLAS Instances Buffer");

    void* mapped = nullptr;
    m_tlasInstances.resource->Map(0, nullptr, &mapped);
    memcpy(mapped, instances.data(), instancesSize);
    m_tlasInstances.resource->Unmap(0, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = static_cast<UINT>(instances.size());
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

    m_result = m_allocator.CreateBuffer(
        prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, "TLAS Result Buffer");

    m_scratch = m_allocator.CreateBuffer(
        prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, "TLAS Scratch Buffer");

    auto commandList = m_commandQueue.GetCommandList();
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
    buildDesc.Inputs = inputs;
    buildDesc.Inputs.InstanceDescs = m_tlasInstances.resource->GetGPUVirtualAddress();
    buildDesc.DestAccelerationStructureData = m_result.resource->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = m_scratch.resource->GetGPUVirtualAddress();
    commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    commandList->ResourceBarrier(1, &barrier);

    m_commandQueue.ExecuteCommandList(commandList);
}

void TLAS::Update(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances) const
{
    void* mapped = nullptr;
    m_tlasInstances.resource->Map(0, nullptr, &mapped);
    memcpy(mapped, instances.data(), instances.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
    m_tlasInstances.resource->Unmap(0, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = static_cast<UINT>(instances.size());
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
        | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
    buildDesc.Inputs = inputs;
    buildDesc.Inputs.InstanceDescs = m_tlasInstances.resource->GetGPUVirtualAddress();
    buildDesc.DestAccelerationStructureData = m_result.resource->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = m_scratch.resource->GetGPUVirtualAddress();
    buildDesc.SourceAccelerationStructureData = m_result.resource->GetGPUVirtualAddress();

    auto commandList = m_commandQueue.GetCommandList();
    commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    commandList->ResourceBarrier(1, &barrier);

    m_commandQueue.ExecuteCommandList(commandList);
}
