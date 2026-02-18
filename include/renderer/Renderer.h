#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <memory>

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
class OutputTexture;
class ShaderCompiler;
class RootSignature;
class RTPipeline;
template<typename T>
class CBVBuffer;

class Renderer
{
public:
	explicit Renderer(Window& window, bool debug);
	~Renderer();
	void Render() const;

	void ToggleFullscreen() const;
	void SetCamera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }

private:
	Window& m_window;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::unique_ptr<Device> m_device = nullptr;

	std::unique_ptr<GPUAllocator> m_allocator = nullptr;
	std::unique_ptr<CommandQueue> m_commandQueue = nullptr;
	std::unique_ptr<DescriptorHeap> m_descriptorHeap = nullptr;
	std::unique_ptr<UploadContext> m_uploadContext = nullptr;
	std::unique_ptr<SwapChain> m_swapChain = nullptr;
	std::unique_ptr<ShaderCompiler> m_shaderCompiler;
	std::unique_ptr<RootSignature> m_rootSignature;
	std::unique_ptr<RTPipeline> m_rtPipeline;

	std::unique_ptr<Model> m_model;
	std::unique_ptr<TLAS> m_tlas;
	std::unique_ptr<CBVBuffer<CameraData>> m_cameraCB;

	std::unique_ptr<OutputTexture> m_rtOutputTexture;
};

