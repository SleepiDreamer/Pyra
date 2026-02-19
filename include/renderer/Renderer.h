#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <memory>
#include "StructsDX.h"

// TODO: remove
#include "Model.h"
#include "TLAS.h"

class Window;
class Camera;
class Device;
class CommandQueue;
class SwapChain;
class DescriptorHeap;
class CommandList;
class UploadContext;
class GPUAllocator;
class OutputBuffer;
class ShaderCompiler;
class RootSignature;
class RTPipeline;
class ImGuiWrapper;
class Scene;
template<typename T>
class CBVBuffer;

class Renderer
{
public:
	explicit Renderer(Window& window, bool debug);
	~Renderer();
	void Render(float deltaTime);

	void ToggleFullscreen() const;
	void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
	void LoadModel(const std::string& path);
	void Resize(int width, int height);

private:
	Window& m_window;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::unique_ptr<Device> m_device = nullptr;

	RenderContext m_context;
	std::unique_ptr<GPUAllocator> m_allocator = nullptr;
	std::unique_ptr<CommandQueue> m_commandQueue = nullptr;
	std::unique_ptr<DescriptorHeap> m_descriptorHeap = nullptr;
	std::unique_ptr<UploadContext> m_uploadContext = nullptr;

	uint64_t m_fenceValues[NUM_FRAMES_IN_FLIGHT] = {};
	std::unique_ptr<SwapChain> m_swapChain = nullptr;
	std::unique_ptr<ShaderCompiler> m_shaderCompiler;
	std::unique_ptr<RootSignature> m_rootSignature;
	std::unique_ptr<RTPipeline> m_rtPipeline;
	std::unique_ptr<ImGuiWrapper> m_imgui = nullptr;

	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<CBVBuffer<CameraData>> m_cameraCB;
	RenderSettings m_renderSettings{};
	std::unique_ptr<CBVBuffer<RenderSettings>> m_renderSettingsCB;

	std::unique_ptr<OutputBuffer> m_rtOutputBuffer;
	std::unique_ptr<OutputBuffer> m_accumulationBuffer;

	float m_reloadTimer = 0.0f;
};

