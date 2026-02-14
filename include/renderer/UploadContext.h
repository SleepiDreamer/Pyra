#pragma once
#include "GPUBuffer.h"
#include "CommandList.h"
#include "CommonDX.h"

class GPUAllocator;
class CommandQueue;

class UploadContext
{
public:
    UploadContext(GPUAllocator& allocator, const CommandQueue& queue,
                  const Microsoft::WRL::ComPtr<ID3D12Device10>& device);
    ~UploadContext();

    void Upload(const GPUBuffer& dest, const void* data, uint64_t size);
    void Flush();
private:
	const CommandQueue& m_queue;
    CommandList m_commandList;
	GPUAllocator& m_allocator;
    std::vector<std::shared_ptr<GPUBuffer>> m_uploads;
};
