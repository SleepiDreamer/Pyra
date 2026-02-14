#pragma once
#include "CommonDX.h"

class RTPipeline
{
public:
	RTPipeline();
	~RTPipeline();
	RTPipeline(const RTPipeline&) = delete;
	RTPipeline& operator=(const RTPipeline&) = delete;

	D3D12_DISPATCH_RAYS_DESC GetDispatchRaysDesc() const;

private:
	Microsoft::WRL::ComPtr<ID3D12StateObject> m_pso;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_raygenTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_missTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_hitGroupTable;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_localRootSignature;
};