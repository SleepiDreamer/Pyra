#include "Renderer.h"
#include "Window.h"
#include "Device.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "SwapChain.h"
#include "DescriptorHeap.h"
#include "UploadContext.h"
#include "CommonDX.h"

#include <iostream>

using namespace Microsoft::WRL;

Renderer::Renderer(Window& window) 
	: m_window(window)
{
	m_device = std::make_unique<Device>(window.GetWidth(), window.GetHeight());

	m_commandQueue = std::make_unique<CommandQueue>(m_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_descriptorHeap = std::make_unique<DescriptorHeap>(m_device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NUM_FRAMES_IN_FLIGHT, true, L"CBV SRV UAV Descriptor Heap");

	m_swapChain = std::make_unique<SwapChain>(window, m_device->GetDevice(), m_device->GetAdapter(), m_commandQueue.get());
}

Renderer::~Renderer()
{
	m_commandQueue->Flush();
}

void Renderer::ToggleFullscreen() const
{
	m_swapChain->ToggleFullscreen();
}

void Renderer::Render()
{
	auto backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_swapChain->GetCurrentBackBuffer();
	auto commandList = m_commandQueue->GetCommandList();
	auto commandQueue = m_commandQueue->GetQueue();

	// Begin frame
	{
		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// Record commands
	{
		constexpr float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(m_swapChain->GetCurrentBackBufferRtv().cpuHandle, clearColor, 0, nullptr);
	}

	// End frame
	{
		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandQueue->ExecuteCommandList(commandList);
		m_swapChain->Present();
	}
}