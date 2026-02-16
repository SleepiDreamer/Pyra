#include "GPUAllocator.h"

GPUAllocator::GPUAllocator(ID3D12Device* device, IDXGIAdapter* adapter)
{
    D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    allocatorDesc.pDevice = device;
    allocatorDesc.pAdapter = adapter;
	ThrowIfFailed(D3D12MA::CreateAllocator(&allocatorDesc, &m_allocator), "Failed to create GPU memory allocator!");
}

GPUAllocator::~GPUAllocator()
{
    if (m_allocator)
    {
        m_allocator->Release();
        m_allocator = nullptr;
    }
}

GPUBuffer GPUAllocator::CreateBuffer(const uint64_t size, const D3D12_RESOURCE_STATES initialState,
									 const D3D12_RESOURCE_FLAGS flags, const D3D12_HEAP_TYPE heapType, const char* name) const
{
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = flags;

    D3D12MA::ALLOCATION_DESC allocDesc{};
    allocDesc.HeapType = heapType;

    GPUBuffer buffer{};
    buffer.size = size;

    ThrowIfFailed(m_allocator->CreateResource(
        &allocDesc,
        &resourceDesc,
        initialState,
        nullptr,
        &buffer.allocation,
        IID_PPV_ARGS(&buffer.resource)));
    buffer.resource->SetName(ToWideString(name).c_str());

    return buffer;
}
