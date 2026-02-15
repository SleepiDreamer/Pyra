#include "SwapChain.h"
#include "Renderer.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "DescriptorHeap.h"
#include "Window.h"
#include "CommonDX.h"

#include <glfw3.h>
#include <algorithm>

using namespace Microsoft::WRL;

SwapChain::SwapChain(Window& window, ID3D12Device* device, IDXGIAdapter4* adapter, const CommandQueue* commandQueue)
	: m_window(window)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window.GetWidth()), static_cast<float>(window.GetHeight()));
	m_scissorRect = CD3DX12_RECT(0, 0, window.GetWidth(), window.GetHeight());

	m_useAdaptiveSync = CheckTearingSupport();
	m_useVsync = !m_useAdaptiveSync;
	m_useHdr = CheckHDRSupport(adapter);

	std::cout << "Tearing support: " << (m_useAdaptiveSync ? "Yes" : "No") << std::endl;
	std::cout << "HDR support: " << (m_useHdr ? "Yes" : "No") << std::endl;

	// Create swap chain
	{
		ComPtr<IDXGIFactory4> factory;
		UINT createFactoryFlags = 0;
#ifdef _DEBUG
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		swapChainDesc.Width = m_window.GetWidth();
		swapChainDesc.Height = m_window.GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = {.Count = 1, .Quality = 0};
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = NUM_FRAMES_IN_FLIGHT;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = m_useAdaptiveSync ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapchain1;
		ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue->GetQueue().Get(), m_window.GetHWND(), &swapChainDesc,
													  nullptr, nullptr, &swapchain1));

		ThrowIfFailed(factory->MakeWindowAssociation(m_window.GetHWND(), DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed(swapchain1.As(&m_swapChain));

		const DXGI_COLOR_SPACE_TYPE colorSpace = m_useHdr
													 ? DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
													 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
		UINT colorSpaceSupport = 0;
		if (SUCCEEDED(m_swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) && (colorSpaceSupport &
			DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
		{
			ThrowIfFailed(m_swapChain->SetColorSpace1(colorSpace));
		}
	}

	m_rtvHeap = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_FRAMES_IN_FLIGHT, false, L"Swap Chain RTV Heap");
	CreateBackBuffers(device);
}

SwapChain::~SwapChain() = default;

bool SwapChain::CheckTearingSupport()
{
	BOOL tearingSupported = FALSE;
	ComPtr<IDXGIFactory5> factory;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		if (FAILED(
			factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupported, sizeof(tearingSupported))))
		{
			tearingSupported = FALSE;
		}
	}

	return tearingSupported == TRUE;
}

bool SwapChain::CheckHDRSupport(IDXGIAdapter4* adapter)
{
	ComPtr<IDXGIOutput> output;
	for (UINT i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		ComPtr<IDXGIOutput6> output6;
		if (SUCCEEDED(output.As(&output6)))
		{
			DXGI_OUTPUT_DESC1 desc;
			ThrowIfFailed(output6->GetDesc1(&desc));

			if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
			{
				return true;
			}
		}
		output.Reset();
	}
	return false;
}

void SwapChain::ToggleFullscreen()
{
	m_fullscreen = !m_fullscreen;
	if (m_fullscreen)
	{
		glfwGetWindowPos(m_window.GetGLFWWindow(), &m_windowedX, &m_windowedY);
		glfwGetWindowSize(m_window.GetGLFWWindow(), &m_windowedWidth, &m_windowedHeight);

		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
		int bestOverlap = 0;

		for (int i = 0; i < monitorCount; i++)
		{
			int mx, my;
			glfwGetMonitorPos(monitors[i], &mx, &my);
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

			int overlapX = max(0, min(m_windowedX + m_windowedWidth, mx + mode->width) - max(m_windowedX, mx));
			int overlapY = max(0, min(m_windowedY + m_windowedHeight, my + mode->height) - max(m_windowedY, my));
			int overlap = overlapX * overlapY;

			if (overlap > bestOverlap)
			{
				bestOverlap = overlap;
				bestMonitor = monitors[i];
			}
		}

		GLFWmonitor* monitor = bestMonitor;
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwSetWindowAttrib(m_window.GetGLFWWindow(), GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(m_window.GetGLFWWindow(), 0, 0);
		glfwSetWindowSize(m_window.GetGLFWWindow(), mode->width, mode->height);

		std::cout << "Entered Borderless Fullscreen: " << mode->width << "x" << mode->height << "\n";
	}
	else
	{
		// Restore windowed mode
		glfwSetWindowAttrib(m_window.GetGLFWWindow(), GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowPos(m_window.GetGLFWWindow(), m_windowedX, m_windowedY);
		glfwSetWindowSize(m_window.GetGLFWWindow(), m_windowedWidth, m_windowedHeight);

		std::cout << "Restored windowed mode: " << m_windowedWidth << "x" << m_windowedHeight << "\n";
	}
}

void SwapChain::CreateBackBuffers(ID3D12Device* device)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = m_format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
		m_backBufferRtvs[i] = m_rtvHeap->Allocate();
		device->CreateRenderTargetView(m_backBuffers[i].Get(), &rtvDesc, m_backBufferRtvs[i].cpuHandle);
		m_backBuffers[i]->SetName(L"Back buffer");
	}
}

void SwapChain::Resize(const uint32_t width, const uint32_t height, ID3D12Device* device)
{
	for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i)
	{
		m_rtvHeap->Free(m_backBufferRtvs[i]);
		m_backBuffers[i].Reset();
	}

	UINT swapChainFlags = m_useAdaptiveSync ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	m_swapChain->ResizeBuffers(NUM_FRAMES_IN_FLIGHT, width, height, m_format, swapChainFlags);
	CreateBackBuffers(device);
}

void SwapChain::Present()
{
	UINT syncInterval = m_useVsync ? 1 : 0;
	UINT presentFlags = m_useAdaptiveSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed(m_swapChain->Present(syncInterval, presentFlags));
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}