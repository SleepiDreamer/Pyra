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
};

