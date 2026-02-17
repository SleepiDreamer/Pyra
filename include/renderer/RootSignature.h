#pragma once
#include "CommonDX.h"

#include <d3dx12.h>
#include <vector>

class RootSignature
{
public:
    RootSignature() = default;
    ~RootSignature() = default;
    RootSignature(const RootSignature&) = delete;
    RootSignature& operator=(const RootSignature&) = delete;

    UINT AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT count, UINT baseRegister, UINT space);

    UINT AddRootSRV(UINT reg, UINT space);
    UINT AddRootUAV(UINT reg, UINT space);
    UINT AddRootCBV(UINT reg, UINT space);

    UINT AddRootConstants(UINT num32BitValues, UINT reg, UINT space);

    void AddStaticSampler(UINT reg, D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
						  D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, UINT space = 0);

    void Build(ID3D12Device* device, const wchar_t* name);

    [[nodiscard]] ID3D12RootSignature* Get() const { return m_rootSignature.Get(); }
    [[nodiscard]] UINT GetParameterCount() const { return static_cast<UINT>(m_params.size()); }

private:
    std::vector<CD3DX12_DESCRIPTOR_RANGE1> m_ranges;
    std::vector<CD3DX12_ROOT_PARAMETER1> m_params;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplers;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
};