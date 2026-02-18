#pragma once
#include "CommonDX.h"

#include <d3dx12.h>
#include <vector>
#include <unordered_map>

class RootSignature
{
public:
    RootSignature() = default;
    ~RootSignature() = default;
    RootSignature(const RootSignature&) = delete;
    RootSignature& operator=(const RootSignature&) = delete;

    UINT AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT count, UINT baseRegister, UINT space, const std::string& name);

    UINT AddRootSRV(UINT reg, UINT space, const std::string& name);
    UINT AddRootUAV(UINT reg, UINT space, const std::string& name);
    UINT AddRootCBV(UINT reg, UINT space, const std::string& name);

    UINT AddRootConstants(UINT num32BitValues, UINT reg, UINT space, const std::string& name);

    void AddStaticSampler(UINT reg, D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
						  D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP, UINT space = 0);

    void Build(ID3D12Device* device, const wchar_t* name);

	void SetDescriptorTable(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE handle, const std::string& name) const;
	void SetRootSRV(ID3D12GraphicsCommandList* commandList, D3D12_GPU_VIRTUAL_ADDRESS address, const std::string& name) const;
	void SetRootUAV(ID3D12GraphicsCommandList* commandList, D3D12_GPU_VIRTUAL_ADDRESS address, const std::string& name) const;
	void SetRootCBV(ID3D12GraphicsCommandList* commandList, D3D12_GPU_VIRTUAL_ADDRESS address, const std::string& name) const;
	void SetRootConstants(ID3D12GraphicsCommandList* commandList, const void* data, UINT num32BitValues, const std::string& name) const;

    [[nodiscard]] ID3D12RootSignature* Get() const { return m_rootSignature.Get(); }
    [[nodiscard]] UINT GetParameterCount() const { return static_cast<UINT>(m_params.size()); }
	[[nodiscard]] UINT GetParameterIndex(const std::string& name) const { return m_paramNames.at(name); }

private:
    std::vector<CD3DX12_DESCRIPTOR_RANGE1> m_ranges;
    std::vector<CD3DX12_ROOT_PARAMETER1> m_params;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplers;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unordered_map<std::string, UINT> m_paramNames;
};