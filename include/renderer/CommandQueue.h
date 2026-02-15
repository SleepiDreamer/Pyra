#pragma once
#include "CommonDX.h"
#include <queue>

// From https://www.3dgep.com/learning-directx-12-2/
class CommandQueue
{
public:
    CommandQueue(const Microsoft::WRL::ComPtr<ID3D12Device10>& device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    virtual ~CommandQueue();

    // Get an available command list from the command queue.
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> GetCommandList();

    // Execute a command list.
    // Returns the fence value to wait for for this command list.
    uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList);

    uint64_t Signal();
    bool IsFenceComplete(uint64_t fenceValue) const;
    void WaitForFenceValue(uint64_t fenceValue) const;
    void Flush();

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetQueue() const;

protected:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() const;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator) const;

private:
    // Keep track of command allocators that are "in-flight"
    struct CommandAllocatorEntry
    {
        uint64_t fenceValue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>>;

    Microsoft::WRL::ComPtr<ID3D12Device10> m_d3d12Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
    D3D12_COMMAND_LIST_TYPE m_commandListType;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
    HANDLE m_FenceEvent;
    uint64_t m_FenceValue = 0;

    CommandAllocatorQueue m_CommandAllocatorQueue;
    CommandListQueue m_CommandListQueue;
};