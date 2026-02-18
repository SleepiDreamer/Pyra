#pragma once
#include "GPUBuffer.h"
#include "GPUAllocator.h"
#include "CommonDX.h"
#include <array>

template<typename T>
class CBVBuffer
{
public:
    CBVBuffer(const GPUAllocator& allocator, const char* name)
    {
        for (int i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        {
            m_buffers[i] = allocator.CreateBuffer(
                sizeof(T),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                D3D12_RESOURCE_FLAG_NONE,
                D3D12_HEAP_TYPE_UPLOAD,
                name);
        }
    }

    void Update(const uint32_t frameIndex, const T& data)
    {
        void* mapped = nullptr;
        m_buffers[frameIndex].resource->Map(0, nullptr, &mapped);
        memcpy(mapped, &data, sizeof(T));
        m_buffers[frameIndex].resource->Unmap(0, nullptr);
    }

    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(const uint32_t frameIndex) const
    {
        return m_buffers[frameIndex].resource->GetGPUVirtualAddress();
    }

private:
    std::array<GPUBuffer, NUM_FRAMES_IN_FLIGHT> m_buffers;
};