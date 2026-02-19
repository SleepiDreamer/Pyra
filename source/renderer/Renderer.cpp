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
#include "RootSignature.h"
#include "RTPipeline.h"
#include "ImGuiWrapper.h"
#include "Scene.h"
#include "CommonDX.h"

#include <imgui.h>
#include <iostream>

using namespace Microsoft::WRL;

Renderer::Renderer(Window& window, bool debug)
	: m_window(window)
{
	m_device = std::make_unique<Device>(window.GetWidth(), window.GetHeight(), debug);
	auto device = m_device->GetDevice();
	m_commandQueue = std::make_unique<CommandQueue>(m_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = m_commandQueue->GetCommandList();
	m_descriptorHeap = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true, L"CBV SRV UAV Descriptor Heap");
	m_allocator = std::make_unique<GPUAllocator>(device, m_device->GetAdapter());
	m_uploadContext = std::make_unique<UploadContext>(*m_allocator, device);

	m_context = { device, m_allocator.get(), m_commandQueue.get(), m_descriptorHeap.get(), m_uploadContext.get() };

	m_swapChain = std::make_unique<SwapChain>(window, device, m_device->GetAdapter(), m_commandQueue.get());
	m_imgui = std::make_unique<ImGuiWrapper>(window, m_context, m_swapChain->GetFormat(), NUM_FRAMES_IN_FLIGHT);

	m_scene = std::make_unique<Scene>(m_context);

	const int width = window.GetWidth();
	const int height = window.GetHeight();
	m_rtOutputTexture = std::make_unique<OutputTexture>(m_context, DXGI_FORMAT_R10G10B10A2_UNORM, width, height, L"RT Output Texture");

	m_shaderCompiler = std::make_unique<ShaderCompiler>();

	m_rootSignature = std::make_unique<RootSignature>();
	m_rootSignature->AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, "outputTexture"); // u0:0 RT output
	m_rootSignature->AddRootSRV(0, 0, "sceneBVH");	// t0:0 TLAS
	m_rootSignature->AddRootSRV(1, 0, "materials"); // t1:0 materials
	m_rootSignature->AddRootCBV(0, 0, "camera");	// b0:0 camera
	m_rootSignature->AddRootCBV(1, 0, "renderSettings");// b1:0 render settings
	m_rootSignature->AddStaticSampler(0);			// s0:0 linear sampler
	m_rootSignature->Build(device, L"RT Root Signature");

	m_rtPipeline = std::make_unique<RTPipeline>(device, m_rootSignature->Get(), *m_shaderCompiler, m_scene->GetHitGroupRecords(), "shaders/raytracing.slang");

	m_cameraCB = std::make_unique<CBVBuffer<CameraData>>(*m_allocator, "Camera CB");
	m_renderSettingsCB = std::make_unique<CBVBuffer<RenderSettings>>(*m_allocator, "Render Settings CB");

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

void Renderer::LoadModel(const std::string& path)
{
	std::cout << "Loading model: " << path;
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	m_scene->LoadModel(path);
	auto time = std::chrono::steady_clock::now() - startTime;
	std::cout << "\r";
	std::cout << "Loaded model: " << path << ". Took " << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() / 1000.0 << " s.\n";

	m_rtPipeline->RebuildShaderTables(m_device->GetDevice(), m_scene->GetHitGroupRecords());
}

void Renderer::Resize(const int width, const int height)
{
	if (width == 0 || height == 0)
	{
		return;
	}

	std::cout << "Window resized: " << width << "x" << height << "\n";
	m_commandQueue->Flush();
	m_swapChain->Resize(width, height, m_device->GetDevice());
	m_rtOutputTexture->Resize(m_device->GetDevice(), width, height);
}

void Renderer::Render(const float deltaTime)
{
	auto backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_swapChain->GetCurrentBackBuffer();
	auto commandList = m_commandQueue->GetCommandList();
	auto commandQueue = m_commandQueue->GetQueue();

	if (m_reloadTimer >= 0.5f)
	{
		m_rtPipeline->CheckHotReload(m_device->GetDevice(), *m_commandQueue, m_scene->GetHitGroupRecords());
		m_reloadTimer = 0.0f;
	}
	else
	{
		m_reloadTimer += deltaTime;
	}

	CameraData camData{};
	camData.forward = m_camera->GetForward();
	camData.right = m_camera->GetRight();
	camData.up = m_camera->GetUp();
	camData.position = m_camera->GetPosition();
	camData.fov = m_camera->m_fov;
	m_cameraCB->Update(backBufferIndex, camData);

	// Begin frame
	{
		m_imgui->BeginFrame();
		m_rtOutputTexture->Transition(commandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// ImGui window
	{
		auto config = ImSettings();
		config.push<float>()
			.as_drag()
			.min(0)
			.max(10)
			.speed(0.02f)
			.pop();
		config.push_member<&RenderSettings::debugMode>()
			.push_member<&RenderSettings::whiteFurnace>()
			.as_input()
			.pop();

		ImGui::Begin("Debug");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame: %d", m_renderSettings.frame);
		ImReflect::Input("Render Settings", m_renderSettings, config);
		ImGui::End();
	}

	m_renderSettings.frame++;
	m_renderSettingsCB->Update(backBufferIndex, m_renderSettings);

	// Record commands
	{
		constexpr float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(m_swapChain->GetCurrentBackBufferRtv().cpuHandle, clearColor, 0, nullptr);

		ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap->GetHeap() };
		commandList->SetDescriptorHeaps(1, heaps);
		commandList->SetComputeRootSignature(m_rootSignature->Get());
		commandList->SetPipelineState1(m_rtPipeline->GetPSO());

		m_rootSignature->SetDescriptorTable(commandList.Get(), m_rtOutputTexture->GetUAV().gpuHandle, "outputTexture");
		m_rootSignature->SetRootSRV(commandList.Get(), m_scene->GetTLASAddress(), "sceneBVH");
		m_rootSignature->SetRootCBV(commandList.Get(), m_cameraCB->GetGPUAddress(backBufferIndex), "camera");
		m_rootSignature->SetRootCBV(commandList.Get(), m_renderSettingsCB->GetGPUAddress(backBufferIndex), "renderSettings");
		m_rootSignature->SetRootSRV(commandList.Get(), m_scene->GetMaterialsBufferAddress(), "materials");

		auto dispatchDesc = m_rtPipeline->GetDispatchRaysDesc();
		dispatchDesc.Width = m_swapChain->GetViewport().Width;
		dispatchDesc.Height = m_swapChain->GetViewport().Height;
		commandList->DispatchRays(&dispatchDesc);
	}

	// End frame
	{
		m_rtOutputTexture->Transition(commandList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->CopyResource(backBuffer, m_rtOutputTexture->GetResource());

		// ImGui
		{
			m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_swapChain->GetCurrentBackBufferRtv().cpuHandle;
			commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

			ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap->GetHeap() };
			commandList->SetDescriptorHeaps(1, heaps);

			m_imgui->EndFrame(commandList.Get());

			m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_PRESENT);
		}

		m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_PRESENT);

		m_fenceValues[backBufferIndex] = m_commandQueue->ExecuteCommandList(commandList);
		m_swapChain->Present();
		m_commandQueue->WaitForFenceValue(m_fenceValues[m_swapChain->GetCurrentBackBufferIndex()]);
	}
}