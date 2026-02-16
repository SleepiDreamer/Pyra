#pragma once
#include "D3D12MemAlloc.h"
#include "GPUBuffer.h"
#include "CommonDX.h"

class GPUAllocator
{
public:
    GPUAllocator(ID3D12Device* device, IDXGIAdapter* adapter);
	~GPUAllocator();

    GPUBuffer CreateBuffer(uint64_t size, D3D12_RESOURCE_STATES initialState,
                           D3D12_RESOURCE_FLAGS flags, D3D12_HEAP_TYPE heapType, const char* name) const;
private:
    D3D12MA::Allocator* m_allocator;
};
