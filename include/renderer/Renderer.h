#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <memory>

class Window;
class Device;
class CommandQueue;
class SwapChain;
class DescriptorHeap;
class CommandList;
class UploadContext;

class Renderer
{
public:
	explicit Renderer(Window& window);
	~Renderer();
	void Render();

	void ToggleFullscreen() const;

private:
	Window& m_window;
	std::unique_ptr<Device> m_device = nullptr;
	std::unique_ptr<CommandQueue> m_commandQueue = nullptr;
	std::unique_ptr<DescriptorHeap> m_descriptorHeap = nullptr;
	std::unique_ptr<UploadContext> m_uploadContext = nullptr;
	std::unique_ptr<SwapChain> m_swapChain = nullptr;
};

