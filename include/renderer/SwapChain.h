#pragma once
#include "DescriptorHeap.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

class Window;
class CommandQueue;

class SwapChain
{
public:
	SwapChain(Window& window, ID3D12Device* device, IDXGIAdapter4* adapter, const CommandQueue* commandQueue);
	~SwapChain();

	[[nodiscard]] static bool CheckTearingSupport();
	[[nodiscard]] static bool CheckHDRSupport(IDXGIAdapter4* adapter);
	[[nodiscard]] UINT GetCurrentBackBufferIndex() const { return m_currentBackBufferIndex; }
	[[nodiscard]] ID3D12Resource* GetCurrentBackBuffer() const { return m_backBuffers[m_currentBackBufferIndex].Get(); }
	[[nodiscard]] DescriptorHeap::Allocation GetCurrentBackBufferRtv() const { return m_backBufferRtvs[m_currentBackBufferIndex]; }
	[[nodiscard]] D3D12_VIEWPORT GetViewport() const { return m_viewport; }
	[[nodiscard]] D3D12_RECT GetScissorRect() const { return m_scissorRect; }

	void CreateBackBuffers(ID3D12Device* device);
	void Resize(uint32_t width, uint32_t height, ID3D12Device* device);
	void Present();

private:
	Window& m_window;
	std::unique_ptr<DescriptorHeap> m_rtvHeap = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain = nullptr;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[NUM_FRAMES_IN_FLIGHT];
	DescriptorHeap::Allocation m_backBufferRtvs[NUM_FRAMES_IN_FLIGHT];
	UINT m_currentBackBufferIndex = 0;
	DXGI_FORMAT m_format = DXGI_FORMAT_R10G10B10A2_UNORM;

	bool m_useAdaptiveSync = false;
	bool m_useVsync = false;
	bool m_useHdr = false;
};
