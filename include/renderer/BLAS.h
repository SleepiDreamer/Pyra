#pragma once
#include "GPUBuffer.h"
#include "CommonDX.h"

class GPUAllocator;
class CommandQueue;

class BLAS
{
public:
    BLAS(GPUAllocator& allocator, CommandQueue& commandQueue);
    ~BLAS();
    BLAS(const BLAS&) = delete;
    BLAS& operator=(const BLAS&) = delete;

    void Build(ID3D12Device5* device, const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometries);
    [[nodiscard]] ID3D12Resource* GetResult() const { return m_result.resource; }

private:
    GPUAllocator& m_allocator;
    CommandQueue& m_commandQueue;
    GPUBuffer m_result;
    GPUBuffer m_scratch;
};