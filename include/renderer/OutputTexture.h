#pragma once
#include "GPUBuffer.h"
#include "DescriptorHeap.h"
#include "CommonDX.h"

class GPUAllocator;

class OutputTexture
{
public:
    OutputTexture(RenderContext& context, DXGI_FORMAT format, int width, int height, std::wstring name);
    ~OutputTexture();

    void Resize(ID3D12Device* device, uint32_t width, uint32_t height);

	void Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState) const;

    [[nodiscard]] ID3D12Resource* GetResource() const { return m_resource.resource; }
    [[nodiscard]] DescriptorHeap::Allocation GetUAV() const { return m_uav; }
    [[nodiscard]] DescriptorHeap::Allocation GetSRV() const { return m_srv; }
    [[nodiscard]] DXGI_FORMAT GetFormat() const { return m_format; }

private:
    void Create(ID3D12Device* device, uint32_t width, uint32_t height);

    RenderContext& m_context;
    GPUBuffer m_resource;
    DescriptorHeap::Allocation m_uav;
    DescriptorHeap::Allocation m_srv;
    DXGI_FORMAT m_format;
    std::wstring m_name;
    bool m_initialized = false;

	mutable D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
};