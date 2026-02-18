#include "Renderer.h"
#include "Window.h"
#include "Camera.h"
#include "Device.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "GPUAllocator.h"
#include "CBVBuffer.h"
#include "UploadContext.h"
#include "SwapChain.h"
#include "OutputTexture.h"
#include "ShaderCompiler.h"
#include "RTPipeline.h"
#include "RootSignature.h"
#include "CommonDX.h"

#include <iostream>

using namespace Microsoft::WRL;

Renderer::Renderer(Window& window, bool debug)
	: m_window(window)
{
	m_device = std::make_unique<Device>(window.GetWidth(), window.GetHeight(), debug);
	auto device = m_device->GetDevice();

	m_commandQueue = std::make_unique<CommandQueue>(m_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = m_commandQueue->GetCommandList();

	m_descriptorHeap = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NUM_FRAMES_IN_FLIGHT, true, L"CBV SRV UAV Descriptor Heap");

	m_allocator = std::make_unique<GPUAllocator>(device, m_device->GetAdapter());

	m_uploadContext = std::make_unique<UploadContext>(*m_allocator, device);

	m_swapChain = std::make_unique<SwapChain>(window, device, m_device->GetAdapter(), m_commandQueue.get());
	const int width = window.GetWidth();
	const int height = window.GetHeight();

	m_model = std::make_unique<Model>(device, *m_commandQueue, *m_allocator, "assets/models/Triangle.gltf");

	m_tlas = std::make_unique<TLAS>(device, *m_allocator, *m_commandQueue);
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances = {};
	auto& meshes = m_model->GetMeshes();
	for (auto& mesh : meshes)
	{
		instances.emplace_back(mesh.GetInstanceDesc());
	}
	m_tlas->Build(device, instances);

	m_rtOutputTexture = std::make_unique<OutputTexture>(device, *m_allocator, *m_descriptorHeap, DXGI_FORMAT_R10G10B10A2_UNORM, width, height, L"RT Output Texture");
	TransitionResource(commandList.Get(), m_rtOutputTexture->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	m_shaderCompiler = std::make_unique<ShaderCompiler>();

	m_rootSignature = std::make_unique<RootSignature>();
	UINT outputSlot = m_rootSignature->AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, "outputTexture"); // u0:0 RT output
	UINT tlasSlot = m_rootSignature->AddRootSRV(0, 0, "sceneBVH"); // t0:0 TLAS
	UINT cameraSlot = m_rootSignature->AddRootCBV(0, 0, "camera"); // b0:0 Camera
	m_rootSignature->Build(device, L"RT Root Signature");

	m_rtPipeline = std::make_unique<RTPipeline>(device, m_rootSignature->Get(), *m_shaderCompiler, "shaders/raytracing.slang");

	m_cameraCB = std::make_unique<CBVBuffer<CameraData>>(*m_allocator, "Camera CB");

	m_commandQueue->ExecuteCommandList(commandList);
	m_commandQueue->Flush();
}

Renderer::~Renderer()
{
	m_commandQueue->Flush();
}

void Renderer::ToggleFullscreen() const
{
	m_swapChain->ToggleFullscreen();
}

void Renderer::Render() const
{
	auto backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_swapChain->GetCurrentBackBuffer();
	auto commandList = m_commandQueue->GetCommandList();
	auto commandQueue = m_commandQueue->GetQueue();

	m_rtPipeline->CheckHotReload(m_device->GetDevice(), *m_commandQueue);

	CameraData camData{};
	camData.forward = m_camera->GetForward();
	camData.right = m_camera->GetRight();
	camData.up = m_camera->GetUp();
	camData.position = m_camera->GetPosition();
	camData.fov = m_camera->m_fov;
	m_cameraCB->Update(backBufferIndex, camData);

	// Begin frame
	{
		TransitionResource(commandList.Get(), m_rtOutputTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// Record commands
	{
		constexpr float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(m_swapChain->GetCurrentBackBufferRtv().cpuHandle, clearColor, 0, nullptr);

		commandList->SetPipelineState1(m_rtPipeline->GetPSO());
		commandList->SetComputeRootSignature(m_rootSignature->Get());

		ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap->GetHeap() };
		commandList->SetDescriptorHeaps(1, heaps);

		m_rootSignature->SetDescriptorTable(commandList.Get(), m_rtOutputTexture->GetUAV().gpuHandle, "outputTexture");
		m_rootSignature->SetRootSRV(commandList.Get(), m_tlas->GetResource().resource->GetGPUVirtualAddress(), "sceneBVH");
		m_rootSignature->SetRootCBV(commandList.Get(), m_cameraCB->GetGPUAddress(backBufferIndex), "camera");

		auto dispatchDesc = m_rtPipeline->GetDispatchRaysDesc();
		dispatchDesc.Width = m_swapChain->GetViewport().Width;
		dispatchDesc.Height = m_swapChain->GetViewport().Height;
		commandList->DispatchRays(&dispatchDesc);
	}

	// End frame
	{
		TransitionResource(commandList.Get(), m_rtOutputTexture->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->CopyResource(backBuffer, m_rtOutputTexture->GetResource());

		TransitionResource(commandList.Get(), backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
		m_commandQueue->ExecuteCommandList(commandList);
		m_swapChain->Present();
	}
}