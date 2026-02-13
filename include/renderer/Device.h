#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class Window;

class Device
{
public:
	explicit Device(const int width, const int height);
	~Device();

	[[nodiscard]] IDXGIAdapter4* GetAdapter() const { return m_adapter.Get(); }
	[[nodiscard]] ID3D12Device10* GetDevice() const { return m_device.Get(); }

private:
	void EnableDebugLayer();
	void CreateAdapter();
	void CreateDevice();

    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
    Microsoft::WRL::ComPtr<ID3D12Device10> m_device;
};

