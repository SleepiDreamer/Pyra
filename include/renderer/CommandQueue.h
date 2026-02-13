#pragma once
#include <d3d12.h>
#include <d3dx12.h>

class Fence;
class CommandList;

class CommandQueue
{
public:
    CommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue();
    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

    [[nodiscard]] ID3D12CommandQueue* GetQueue() const { return m_queue.Get(); }
    [[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const { return m_type; }
    [[nodiscard]] const Fence& GetFence() const { return *m_fence; }

    uint64_t Execute(CommandList& commandList, uint32_t frameIndex) const;
    void GPUSynchronize(const CommandQueue& queue) const;
    void Flush() const;

private:
    friend class Fence;
    friend class CommandList;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue = nullptr;
    D3D12_COMMAND_LIST_TYPE m_type;
    std::unique_ptr<Fence> m_fence = nullptr;
};