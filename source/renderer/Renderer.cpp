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
#include "PostProcessPass.h"
#include "ImGuiWrapper.h"
#include "Scene.h"
#include "StructsDX.h"
#include "CommonDX.h"

#include <imgui.h>
#include <iostream>
#include <chrono>

using namespace Microsoft::WRL;

// TODO
// - explicit lights
// - DLSS SR & RR

Renderer::Renderer(Window& window, bool debug)
	: m_window(window), m_prevCamData()
{
	m_device = std::make_unique<Device>(window.GetWidth(), window.GetHeight(), debug);
	auto device = m_device->GetDevice();
	m_commandQueue = std::make_unique<CommandQueue>(m_device->GetDevice(), "Main", D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = m_commandQueue->GetCommandList();
	m_descriptorHeap = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true, L"CBV SRV UAV Descriptor Heap");
	m_allocator = std::make_unique<GPUAllocator>(device, m_device->GetAdapter());
	m_uploadContext = std::make_unique<UploadContext>(*m_allocator, device);

	m_context = { device, m_allocator.get(), m_commandQueue.get(), m_descriptorHeap.get(), m_uploadContext.get() };

	m_swapChain = std::make_unique<SwapChain>(window, device, m_device->GetAdapter(), m_commandQueue.get());
	m_imgui = std::make_unique<ImGuiWrapper>(window, m_context, m_swapChain->GetFormat(), NUM_FRAMES_IN_FLIGHT);

	m_scene = std::make_unique<Scene>(m_context);

	m_shaderCompiler = std::make_unique<ShaderCompiler>();

	const int width = window.GetWidth();
	const int height = window.GetHeight();
	m_accumulationBuffer = std::make_unique<OutputBuffer>(m_context, DXGI_FORMAT_R32G32B32A32_FLOAT, width, height, L"Accumulation Buffer");
	m_outputBuffer = std::make_unique<OutputBuffer>(m_context, DXGI_FORMAT_R10G10B10A2_UNORM, width, height, L"RT Output Buffer");

	m_rootSignature = std::make_unique<RootSignature>();
	m_rootSignature->AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, "accumulationBuffer"); // u0:0 accumulation buffer
	m_rootSignature->AddRootSRV(0, 0, "sceneBVH");			 // t0:0 TLAS
	m_rootSignature->AddRootSRV(1, 0, "materials");			 // t1:0 materials
	m_rootSignature->AddRootCBV(0, 0, "camera");			 // b0:0 camera
	m_rootSignature->AddRootCBV(1, 0, "renderSettings");	 // b1:0 render settings
	m_rootSignature->AddRootCBV(2, 0, "renderData");		 // b2:0 render data
	m_rootSignature->AddRootCBV(3, 0, "postProcessSettings");// b3:0 post processing settings
	m_rootSignature->AddStaticSampler(0);					 // s0:0 linear sampler
	m_rootSignature->Build(device, L"RT Root Signature");

	m_rtPipeline = std::make_unique<RTPipeline>(device, m_rootSignature->Get(), *m_shaderCompiler,
	                                            m_scene->GetHitGroupRecords(), "shaders/raytracing.slang");

	m_tonemappingPass = std::make_unique<PostProcessPass>(m_context, *m_shaderCompiler, "shaders/tonemapping_pass.slang", "CSMain");

	m_cameraCB = std::make_unique<CBVBuffer<CameraData>>(*m_allocator, "Camera CB");
	m_renderSettingsCB = std::make_unique<CBVBuffer<RenderSettings>>(*m_allocator, "Render Settings CB");
	m_renderDataCB = std::make_unique<CBVBuffer<RenderData>>(*m_allocator, "Render Data CB");
	m_postProcessSettingsCB = std::make_unique<CBVBuffer<PostProcessSettings>>(*m_allocator, "Post Process Settings CB");

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
	if (m_scene->LoadModel(path))
	{
		m_rtPipeline->RebuildShaderTables(m_device->GetDevice(), m_scene->GetHitGroupRecords());
		ResetAccumulation();
	}
}

void Renderer::LoadHDRI(const std::string& path)
{
	m_scene->LoadHDRI(path);
	ResetAccumulation();
}

void Renderer::Resize(const int width, const int height)
{
	if (width == 0 || height == 0)
	{
		return;
	}

	std::cout << "Window resized: " << width << "x" << height << "\n";
	ResetAccumulation();
	m_commandQueue->Flush();
	m_swapChain->Resize(width, height, m_device->GetDevice());
	m_outputBuffer->Resize(m_device->GetDevice(), width, height);
	m_accumulationBuffer->Resize(m_device->GetDevice(), width, height);
}

void Renderer::Render(const float deltaTime)
{
	auto backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_swapChain->GetCurrentBackBuffer();
	auto commandList = m_commandQueue->GetCommandList();
	auto commandQueue = m_commandQueue->GetQueue();

	if (m_reloadTimer >= 0.5f)
	{
		if (m_rtPipeline->CheckHotReload(m_device->GetDevice(), *m_commandQueue, m_scene->GetHitGroupRecords()))
		{
			ResetAccumulation();
		}
		m_tonemappingPass->CheckHotReload(*m_commandQueue);
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

	// Begin frame
	{
		m_imgui->BeginFrame();
		m_outputBuffer->Transition(commandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	// ImGui window
	{
		if (!m_rtPipeline->IsLastCompileSuccesful())
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
			ImGui::Begin("Border", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);
			ImGui::End();
			ImGui::PopStyleColor();

			ImGui::SetNextWindowPos(ImVec2(m_window.GetWidth() / 2.0f - 400.0f, m_window.GetHeight() / 2.0f - 200.0f));
			ImGui::Begin("Shader Compile Errors", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PushTextWrapPos(800.0f);
			ImGui::TextWrapped("%s", m_rtPipeline->GetLastCompileError().c_str());
			ImGui::PopTextWrapPos();
			ImGui::End();
		}

		auto config = ImSettings();
		config.push<float>().as_drag().min(0).max(10).speed(0.02f).pop();
		auto config2 = ImSettings();
		config2.push<float>().as_drag().min(0).max(100).speed(0.02f).pop();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Debug");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame: %u", m_renderData.frame);
		auto responseRender = ImReflect::Input("Render Settings", m_renderSettings, config);
		auto responsePost = ImReflect::Input("Post Process Settings", m_postProcessSettings, config);
		auto responseCamera = ImReflect::Input("Camera", camData, config2);
		if (responseRender.get<RenderSettings>().is_changed())
		{
			ResetAccumulation();
		}
		if (responseCamera.get<CameraData>().is_changed())
		{
			ResetAccumulation();
			m_camera->SetPosition(camData.position);
			m_camera->SetDirection(camData.forward);
			m_camera->m_fov = camData.fov;
		}
		
		ImGui::End();
	}

	m_renderData.hdriIndex = m_scene->GetHDRIDescriptorIndex();
	m_renderSettingsCB->Update(backBufferIndex, m_renderSettings);
	m_renderDataCB->Update(backBufferIndex, m_renderData);
	m_postProcessSettingsCB->Update(backBufferIndex, m_postProcessSettings);
	m_cameraCB->Update(backBufferIndex, camData);

	// Record commands
	{
		// Raytracing pass
		{
			ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap->GetHeap() };
			commandList->SetDescriptorHeaps(1, heaps);
			commandList->SetComputeRootSignature(m_rootSignature->Get());
			commandList->SetPipelineState1(m_rtPipeline->GetPSO());

			m_rootSignature->SetDescriptorTable(commandList.Get(), m_accumulationBuffer->GetUAV().gpuHandle, "accumulationBuffer");
			m_rootSignature->SetRootSRV(commandList.Get(), m_scene->GetTLASAddress(), "sceneBVH");
			m_rootSignature->SetRootCBV(commandList.Get(), m_cameraCB->GetGPUAddress(backBufferIndex), "camera");
			m_rootSignature->SetRootCBV(commandList.Get(), m_renderSettingsCB->GetGPUAddress(backBufferIndex), "renderSettings");
			m_rootSignature->SetRootCBV(commandList.Get(), m_renderDataCB->GetGPUAddress(backBufferIndex), "renderData");
			m_rootSignature->SetRootCBV(commandList.Get(), m_postProcessSettingsCB->GetGPUAddress(backBufferIndex), "postProcessSettings");
			m_rootSignature->SetRootSRV(commandList.Get(), m_scene->GetMaterialsBufferAddress(), "materials");

			auto dispatchDesc = m_rtPipeline->GetDispatchRaysDesc();
			dispatchDesc.Width = static_cast<UINT>(m_swapChain->GetViewport().Width);
			dispatchDesc.Height = static_cast<UINT>(m_swapChain->GetViewport().Height);
			commandList->DispatchRays(&dispatchDesc);
		}

		// Tonemapping pass
		{
			m_accumulationBuffer->Transition(commandList.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_outputBuffer->Transition(commandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			PostProcessPass::PostProcessBindings bindings;
			bindings.inputSRV = m_accumulationBuffer->GetSRV().gpuHandle;
			bindings.outputUAV = m_outputBuffer->GetUAV().gpuHandle;
			bindings.constants[0] = m_renderSettingsCB->GetGPUAddress(backBufferIndex);
			bindings.constants[1] = m_renderDataCB->GetGPUAddress(backBufferIndex);
			bindings.constants[2] = m_postProcessSettingsCB->GetGPUAddress(backBufferIndex);
			bindings.constantCount = 3;
			bindings.width = static_cast<uint32_t>(m_swapChain->GetViewport().Width);
			bindings.height = static_cast<uint32_t>(m_swapChain->GetViewport().Height);

			m_tonemappingPass->Dispatch(commandList.Get(), bindings);

			m_accumulationBuffer->Transition(commandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}

	// End frame
	{
		m_outputBuffer->Transition(commandList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->CopyResource(backBuffer, m_outputBuffer->GetResource());

		// ImGui
		{
			m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_swapChain->GetCurrentBackBufferRtv().cpuHandle;
			commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

			ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap->GetHeap() };
			commandList->SetDescriptorHeaps(1, heaps);

			m_imgui->EndFrame(commandList.Get());
		}

		m_swapChain->Transition(commandList.Get(), D3D12_RESOURCE_STATE_PRESENT);

		m_fenceValues[backBufferIndex] = m_commandQueue->ExecuteCommandList(commandList);
		m_swapChain->Present();
		m_commandQueue->WaitForFenceValue(m_fenceValues[m_swapChain->GetCurrentBackBufferIndex()]);
		
		m_renderData.frame++;
		if (camData.position != m_prevCamData.position || camData.forward != m_prevCamData.forward)
		{
			ResetAccumulation();
		}
		m_prevCamData = camData;
	}
}