#include "RTPipeline.h"
#include "ShaderCompiler.h"
#include "ShaderLibrary.h"
#include "CommandQueue.h"
#include "HelpersDX.h"

#include <d3dx12.h>
#include <iostream>

RTPipeline::RTPipeline(ID3D12Device10* device, ShaderCompiler& compiler,
    const std::string& shaderPath)
{
	std::vector<std::string> entryPoints = {}; // entry points found automatically with attributes
    m_shaderLibrary = std::make_unique<ShaderLibrary>(compiler, shaderPath, entryPoints);
    if (!m_shaderLibrary->IsValid())
    {
	    throw std::runtime_error("Failed to compile RT shaders: " + shaderPath);
    }

    CreateRootSignature(device);
    CreatePSO(device);
    CreateShaderTables(device);
}

RTPipeline::~RTPipeline() = default;

void RTPipeline::CreateRootSignature(ID3D12Device10* device)
{
    CD3DX12_DESCRIPTOR_RANGE1 uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

    CD3DX12_ROOT_PARAMETER1 params[3] = {};
    params[0].InitAsDescriptorTable(1, &uavRange);
    params[1].InitAsShaderResourceView(0); // t0: TLAS
    params[2].InitAsConstantBufferView(0); // b0: constants

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
    desc.Init_1_1(_countof(params), params, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&desc,
        D3D_ROOT_SIGNATURE_VERSION_1_1, &sigBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
            std::cerr << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
        ThrowIfFailed(hr);
    }

    ThrowIfFailed(device->CreateRootSignature(0, sigBlob->GetBufferPointer(),
        sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void RTPipeline::CreatePSO(ID3D12Device10* device)
{
    CD3DX12_STATE_OBJECT_DESC psoDesc(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

    auto lib = psoDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    auto bytecode = m_shaderLibrary->GetBytecode();
    lib->SetDXILLibrary(&bytecode);

    // Hit group
    auto hitGroup = psoDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(L"ClosestHit");
    hitGroup->SetHitGroupExport(L"HitGroup");
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto shaderConfig = psoDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    shaderConfig->Config(
        sizeof(float) * 3,  // max payload size
        sizeof(float) * 2   // max attribute size
    );

    auto globalRootSig = psoDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSig->SetRootSignature(m_rootSignature.Get());

    auto pipelineConfig = psoDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    pipelineConfig->Config(2); // max recursion depth

    ThrowIfFailed(device->CreateStateObject(psoDesc, IID_PPV_ARGS(&m_pso)));
}

void RTPipeline::CreateShaderTables(ID3D12Device10* device)
{
    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> props;
    ThrowIfFailed(m_pso.As(&props));

    constexpr UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    auto Align = [](UINT size, UINT alignment) -> UINT
    {
        return (size + alignment - 1) & ~(alignment - 1);
    };

    m_raygenRecordSize = Align(shaderIdSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    m_missRecordSize = Align(shaderIdSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    m_hitGroupRecordSize = Align(shaderIdSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    m_hitGroupCount = 1; // number of meshes

    // Raygen table
    {
        UINT tableSize = Align(m_raygenRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
        ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					  &bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_raygenTable)));

        void* mapped = nullptr;
        m_raygenTable->Map(0, nullptr, &mapped);
        memcpy(mapped, props->GetShaderIdentifier(L"RayGen"), shaderIdSize);
        m_raygenTable->Unmap(0, nullptr);
    }

    // Miss table
    {
        UINT tableSize = Align(m_missRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
        ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					  &bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_missTable)));

        void* mapped = nullptr;
        m_missTable->Map(0, nullptr, &mapped);
        memcpy(mapped, props->GetShaderIdentifier(L"Miss"), shaderIdSize);
        m_missTable->Unmap(0, nullptr);
    }

    // Hit group table
    {
        UINT tableSize = Align(m_hitGroupRecordSize * m_hitGroupCount,
            D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
        ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					  &bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_hitGroupTable)));

        void* mapped = nullptr;
        m_hitGroupTable->Map(0, nullptr, &mapped);
        auto* dst = static_cast<uint8_t*>(mapped);
        memcpy(dst, props->GetShaderIdentifier(L"HitGroup"), shaderIdSize);
        m_hitGroupTable->Unmap(0, nullptr);
    }
}

void RTPipeline::Rebuild(ID3D12Device10* device)
{
    m_pso.Reset();
    m_raygenTable.Reset();
    m_missTable.Reset();
    m_hitGroupTable.Reset();

    CreatePSO(device);
    CreateShaderTables(device);
}

void RTPipeline::RebuildShaderTables(ID3D12Device10* device)
{
    m_raygenTable.Reset();
    m_missTable.Reset();
    m_hitGroupTable.Reset();
    CreateShaderTables(device);
}

bool RTPipeline::CheckHotReload(ID3D12Device10* device, CommandQueue& commandQueue)
{
    if (!m_shaderLibrary->NeedsReload())
        return false;

    if (!m_shaderLibrary->Reload())
    {
        std::cerr << "[RTPipeline] Hot reload failed, keeping previous shaders\n";
        return false;
    }

    commandQueue.Flush();
    Rebuild(device);
    std::cout << "[RTPipeline] Hot reload successful\n";
    return true;
}

D3D12_DISPATCH_RAYS_DESC RTPipeline::GetDispatchRaysDesc() const
{
    D3D12_DISPATCH_RAYS_DESC desc{};

    desc.RayGenerationShaderRecord.StartAddress = m_raygenTable->GetGPUVirtualAddress();
    desc.RayGenerationShaderRecord.SizeInBytes = m_raygenRecordSize;

    desc.MissShaderTable.StartAddress = m_missTable->GetGPUVirtualAddress();
    desc.MissShaderTable.SizeInBytes = m_missRecordSize;
    desc.MissShaderTable.StrideInBytes = m_missRecordSize;

    desc.HitGroupTable.StartAddress = m_hitGroupTable->GetGPUVirtualAddress();
    desc.HitGroupTable.SizeInBytes = m_hitGroupCount * m_hitGroupRecordSize;
    desc.HitGroupTable.StrideInBytes = m_hitGroupRecordSize;

    desc.Width = 0;
    desc.Height = 0;
    desc.Depth = 1;

    return desc;
}