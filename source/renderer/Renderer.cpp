#include "Renderer.h"
#include "Window.h"
#include "Device.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "GPUAllocator.h"
#include "UploadContext.h"
#include "SwapChain.h"
#include "OutputTexture.h"
#include "ShaderCompiler.h"
#include "RTPipeline.h"
#include "CommonDX.h"

#include <iostream>

using namespace Microsoft::WRL;

Renderer::Renderer(Window& window) 
	: m_window(window)
{
	m_device = std::make_unique<Device>(window.GetWidth(), window.GetHeight());

	m_commandQueue = std::make_unique<CommandQueue>(m_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_descriptorHeap = std::make_unique<DescriptorHeap>(m_device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NUM_FRAMES_IN_FLIGHT, true, L"CBV SRV UAV Descriptor Heap");

	m_allocator = std::make_unique<GPUAllocator>(m_device->GetDevice(), m_device->GetAdapter());

	m_uploadContext = std::make_unique<UploadContext>(*m_allocator, m_device->GetDevice());

	m_swapChain = std::make_unique<SwapChain>(window, m_device->GetDevice(), m_device->GetAdapter(), m_commandQueue.get());

	m_model = std::make_unique<Model>(m_device->GetDevice(),*m_commandQueue, *m_allocator, "assets/models/Triangle.gltf");

	m_tlas = std::make_unique<TLAS>(m_device->GetDevice(), *m_allocator, *m_commandQueue);

	m_rtOutputTexture = std::make_unique<OutputTexture>(m_device->GetDevice(), *m_allocator, *m_descriptorHeap, DXGI_FORMAT_R16G16B16A16_FLOAT, L"RT Output Texture");

	m_shaderCompiler = std::make_unique<ShaderCompiler>();

	m_rtPipeline = std::make_unique<RTPipeline>(m_device->GetDevice(), *m_shaderCompiler, "shaders/raytracing.slang");
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

	m_rtPipeline->CheckHotReload(m_device->GetDevice(), *m_commandQueue);

	// Begin frame
	{
		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// Record commands
	{
		constexpr float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(m_swapChain->GetCurrentBackBufferRtv().cpuHandle, clearColor, 0, nullptr);

		commandList->SetPipelineState1(m_rtPipeline->GetPSO());
		commandList->SetComputeRootSignature(m_rtPipeline->GetRootSignature());

		auto dispatchDesc = m_rtPipeline->GetDispatchRaysDesc();
		dispatchDesc.Width = m_swapChain->GetViewport().Width;
		dispatchDesc.Height = m_swapChain->GetViewport().Height;
		commandList->DispatchRays(&dispatchDesc);
	}

	// End frame
	{
		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandQueue->ExecuteCommandList(commandList);
		m_swapChain->Present();
	}
}