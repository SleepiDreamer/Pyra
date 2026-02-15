#include "UploadContext.h"
#include "GPUAllocator.h"
#include "CommandQueue.h"
#include "HelpersDX.h"

UploadContext::UploadContext(GPUAllocator& allocator, const CommandQueue& queue,
    const Microsoft::WRL::ComPtr<ID3D12Device10>& device)
    : m_queue(queue)
    , m_allocator(allocator)
{
}

UploadContext::~UploadContext()
{
    Flush();
}

void UploadContext::Upload(const GPUBuffer& dest, const void* data, const uint64_t size)
{
    const auto staging = std::make_shared<GPUBuffer>(
			m_allocator.CreateBuffer(size, D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD));

    void* mapped = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    ThrowIfFailed(staging->resource->Map(0, &readRange, &mapped));
    memcpy(mapped, data, size);
    staging->resource->Unmap(0, nullptr);

    // TODO: implement
    //const auto commandList = m_commandList.GetCommandList();
    //commandList->CopyBufferRegion(dest.resource, 0, staging->resource, 0, size);

    m_uploads.push_back(staging);
}

void UploadContext::Flush()
{
    if (m_uploads.empty())
        return;

    // TODO: implement
}