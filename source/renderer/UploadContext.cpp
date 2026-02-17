#include "UploadContext.h"
#include "GPUAllocator.h"
#include "CommandQueue.h"
#include "HelpersDX.h"

UploadContext::UploadContext(GPUAllocator& allocator, ID3D12Device10* device)
    : m_allocator(allocator)
{
    m_queue = std::make_unique<CommandQueue>(device, D3D12_COMMAND_LIST_TYPE_COPY);
}

UploadContext::~UploadContext()
{
    Flush();
}

void UploadContext::Upload(const GPUBuffer& dest, const void* data, const uint64_t size)
{
    GPUBuffer staging = m_allocator.CreateBuffer(
        size, D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, "Upload Staging Buffer");

    void* mapped = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    ThrowIfFailed(staging.resource->Map(0, &readRange, &mapped));
    memcpy(mapped, data, size);
    staging.resource->Unmap(0, nullptr);

    auto cmdList = m_queue->GetCommandList();
    cmdList->CopyBufferRegion(dest.resource, 0, staging.resource, 0, size);
    m_queue->ExecuteCommandList(cmdList);

    m_uploads.push_back(std::move(staging));
}

void UploadContext::Flush()
{
    if (m_uploads.empty())
        return;

    m_queue->Flush();
    m_uploads.clear();
}