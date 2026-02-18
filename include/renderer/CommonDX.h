#pragma once
#include "StructsDX.h"
#include "HelpersDX.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

inline constexpr uint8_t NUM_FRAMES_IN_FLIGHT = 3;

constexpr DXGI_SAMPLE_DESC DEFAULT_SAMPLER = { 1, 0 };
constexpr D3D12_RESOURCE_DESC BUFFER_RESOURCE = {
    D3D12_RESOURCE_DIMENSION_BUFFER,
    0,
    0,
    1,
    1,
    1,
    DXGI_FORMAT_UNKNOWN,
    DEFAULT_SAMPLER,
    D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    D3D12_RESOURCE_FLAG_NONE
};
constexpr D3D12_RESOURCE_DESC TEXTURE_RESOURCE = {
    D3D12_RESOURCE_DIMENSION_TEXTURE2D,
    0,
    0,
    1,
    1,
    1,
    DXGI_FORMAT_UNKNOWN,
    DEFAULT_SAMPLER,
    D3D12_TEXTURE_LAYOUT_UNKNOWN,
    D3D12_RESOURCE_FLAG_NONE
};