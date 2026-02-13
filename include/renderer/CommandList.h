#pragma once
#include "SwapChain.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <array>

class Fence;

class CommandList
{
public:
    CommandList(const Microsoft::WRL::ComPtr<ID3D12Device10>& device, D3D12_COMMAND_LIST_TYPE type);
    ~CommandList() = default;
    CommandList(const CommandList&) = delete;
    CommandList& operator=(const CommandList&) = delete;

    [[nodiscard]] ID3D12GraphicsCommandList6* GetCommandList() const { return m_commandList.Get(); }

    void Reset(uint32_t frameIndex, const Fence& fence);
    void Close();
    void StartMarker(const std::string& name) const;
    void EndMarker() const;

private:
    friend class CommandQueue;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> m_commandList = nullptr;
    std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, SwapChain::m_numBuffering> m_allocators{};
    std::array<uint64_t, SwapChain::m_numBuffering> m_allocatorFenceValues{};
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator = nullptr;
    D3D12_COMMAND_LIST_TYPE m_type;
    bool m_cmdListClosed = false;
};
