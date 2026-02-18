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

void UploadContext::UploadTexture(const GPUBuffer& dest, const void* data, const uint32_t width, const uint32_t height, const DXGI_FORMAT format)
{
    // Calculate row sizes
    UINT bytesPerPixel = 0;
    switch (format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        bytesPerPixel = 4;
        break;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        bytesPerPixel = 16;
        break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        bytesPerPixel = 8;
        break;
    case DXGI_FORMAT_R8_UNORM:
        bytesPerPixel = 1;
        break;
    case DXGI_FORMAT_R8G8_UNORM:
        bytesPerPixel = 2;
        break;
    default:
        ThrowError("Unsupported texture format for upload");
    }

    UINT srcRowPitch = width * bytesPerPixel;
	// 256 byte alignment required for texture uploads
    UINT alignedRowPitch = (srcRowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
        & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
    UINT64 stagingSize = static_cast<UINT64>(alignedRowPitch) * height;

    auto staging = m_allocator.CreateBuffer(stagingSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD, "Texture Upload Staging Buffer");

    void* mapped = nullptr;
    D3D12_RANGE readRange{ 0, 0 };
    ThrowIfFailed(staging.resource->Map(0, &readRange, &mapped));

    auto* srcBytes = static_cast<const uint8_t*>(data);
    auto* dstBytes = static_cast<uint8_t*>(mapped);
    for (UINT row = 0; row < height; row++)
    {
        memcpy(dstBytes + row * alignedRowPitch,
            srcBytes + row * srcRowPitch,
            srcRowPitch);
    }

    staging.resource->Unmap(0, nullptr);

    D3D12_TEXTURE_COPY_LOCATION srcLoc{};
    srcLoc.pResource = staging.resource;
    srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLoc.PlacedFootprint.Offset = 0;
    srcLoc.PlacedFootprint.Footprint.Format = format;
    srcLoc.PlacedFootprint.Footprint.Width = width;
    srcLoc.PlacedFootprint.Footprint.Height = height;
    srcLoc.PlacedFootprint.Footprint.Depth = 1;
    srcLoc.PlacedFootprint.Footprint.RowPitch = alignedRowPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLoc{};
    dstLoc.pResource = dest.resource;
    dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex = 0;

    auto cmdList = m_queue->GetCommandList();
    cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
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
