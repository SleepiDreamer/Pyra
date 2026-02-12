#pragma once
#include <memory>

class Window;

class Renderer
{
public:
	Renderer(Window& window);
	~Renderer();
	void Render();
	

private:
	Window& m_window;
	//std::unique_ptr<Device> m_device = nullptr;
};

