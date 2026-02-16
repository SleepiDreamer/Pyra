#pragma once
#include "CommonDX.h"

class ShaderCompiler;
class ShaderLibrary;
class CommandQueue;

class RTPipeline
{
public:
    RTPipeline(ID3D12Device10* device, ShaderCompiler& compiler, const std::string& shaderPath);
    ~RTPipeline();

    void Rebuild(ID3D12Device10* device);
    void RebuildShaderTables(ID3D12Device10* device);
    bool CheckHotReload(ID3D12Device10* device, CommandQueue& commandQueue);
    D3D12_DISPATCH_RAYS_DESC GetDispatchRaysDesc() const;
    ID3D12StateObject* GetPSO() const { return m_pso.Get(); }
    ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }

private:
    void CreateRootSignature(ID3D12Device10* device);
    void CreatePSO(ID3D12Device10* device);
    void CreateShaderTables(ID3D12Device10* device);

    Microsoft::WRL::ComPtr<ID3D12StateObject> m_pso;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_localRootSignature;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_raygenTable;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_missTable;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_hitGroupTable;

    UINT m_raygenRecordSize = 0;
    UINT m_missRecordSize = 0;
    UINT m_hitGroupRecordSize = 0;
    UINT m_hitGroupCount = 0;

    std::unique_ptr<ShaderLibrary> m_shaderLibrary;
};