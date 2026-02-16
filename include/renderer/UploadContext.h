#pragma once
#include "GPUBuffer.h"
#include "CommonDX.h"

class GPUAllocator;
class CommandQueue;

class UploadContext
{
public:
    UploadContext(GPUAllocator& allocator, ID3D12Device10* device);
    ~UploadContext();

    void Upload(const GPUBuffer& dest, const void* data, uint64_t size);
    void Flush();
private:
	GPUAllocator& m_allocator;
    std::unique_ptr<CommandQueue> m_queue;
    std::vector<std::shared_ptr<GPUBuffer>> m_uploads;
};
