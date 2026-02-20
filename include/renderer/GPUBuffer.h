#pragma once
#include "CommonDX.h"
#include <D3D12MemAlloc.h>

struct GPUBuffer
{
    D3D12MA::Allocation* allocation = nullptr;
    ID3D12Resource* resource = nullptr;
    uint64_t size = 0;

    GPUBuffer() = default;
    ~GPUBuffer()
    {
        Reset();
    }

    // Move only
    GPUBuffer(const GPUBuffer&) = delete;
    GPUBuffer& operator=(const GPUBuffer&) = delete;

    GPUBuffer(GPUBuffer&& other) noexcept
        : allocation(other.allocation), resource(other.resource), size(other.size)
    {
        other.allocation = nullptr;
        other.resource = nullptr;
        other.size = 0;
    }

    GPUBuffer& operator=(GPUBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            allocation = other.allocation;
            resource = other.resource;
            size = other.size;
            other.allocation = nullptr;
            other.resource = nullptr;
            other.size = 0;
        }
        return *this;
    }

    void Reset()
    {
        if (allocation)
        {
            allocation->Release();
            allocation = nullptr;
            resource->Release();
            resource = nullptr;
            size = 0;
        }
    }

    explicit operator bool() const { return allocation != nullptr; }
};