#pragma once
#include "CommonDX.h"
#include "D3D12MemAlloc.h"

struct GPUBuffer
{
    D3D12MA::Allocation* allocation = nullptr;
    ID3D12Resource* resource = nullptr;
    uint64_t size = 0;
};