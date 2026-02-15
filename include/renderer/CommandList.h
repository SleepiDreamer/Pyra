//#pragma once
//#include "CommonDX.h"
//
//#include <d3d12.h>
//#include <d3dx12.h>
//#include <array>
//
//class Fence;
//
//class CommandList
//{
//public:
//    CommandList(const Microsoft::WRL::ComPtr<ID3D12Device10>& device, D3D12_COMMAND_LIST_TYPE type);
//    ~CommandList() = default;
//
//    [[nodiscard]] ID3D12GraphicsCommandList6* GetCommandList() const { return m_commandList.Get(); }
//	[[nodiscard]] bool IsClosed() const { return m_cmdListClosed; }
//
//    void Reset(uint32_t frameIndex, const Fence& fence);
//    void Close();
//    void StartMarker(const std::string& name) const;
//    void EndMarker() const;
//
//private:
//    friend class CommandQueue;
//
//    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> m_commandList = nullptr;
//    D3D12_COMMAND_LIST_TYPE m_type;
//    bool m_cmdListClosed = false;
//};
