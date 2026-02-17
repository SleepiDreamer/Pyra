#pragma once
#include "GPUBuffer.h"
#include "DescriptorHeap.h"
#include "CommonDX.h"

class GPUAllocator;

class OutputTexture
{
public:
    OutputTexture(ID3D12Device* device, GPUAllocator& allocator, DescriptorHeap& descriptorHeap, DXGI_FORMAT format, int width, int height, std::wstring name);
    ~OutputTexture();

    void Resize(ID3D12Device* device, uint32_t width, uint32_t height);

    [[nodiscard]] ID3D12Resource* GetResource() const { return m_resource.resource; }
    [[nodiscard]] DescriptorHeap::Allocation GetUAV() const { return m_uav; }
    [[nodiscard]] DescriptorHeap::Allocation GetSRV() const { return m_srv; }
    [[nodiscard]] DXGI_FORMAT GetFormat() const { return m_format; }

private:
    void Create(ID3D12Device* device, uint32_t width, uint32_t height);

    GPUAllocator& m_allocator;
    DescriptorHeap& m_descriptorHeap;
    GPUBuffer m_resource;
    DescriptorHeap::Allocation m_uav;
    DescriptorHeap::Allocation m_srv;
    DXGI_FORMAT m_format;
    std::wstring m_name;
    bool m_initialized = false;
};