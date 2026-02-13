#pragma once
#include "d3d12.h"
#include "d3dx12.h"

class CommandQueue;

class Fence
{
public:
	explicit Fence(ID3D12Device2* device);
    ~Fence();

    void Signal(const CommandQueue& queue);
    void Wait() const;
    void WaitFor(uint64_t value) const;

	[[nodiscard]] uint64_t GetWaitValue() const;
	[[nodiscard]] uint64_t GetCurrentValue() const;
    [[nodiscard]] bool IsComplete(uint64_t value) const;

private:
    friend class CommandQueue;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence = nullptr;
    uint64_t m_fenceValue = 0;
    HANDLE m_event = nullptr;
};