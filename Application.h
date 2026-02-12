#pragma once
#include <memory>

class Window;
class Renderer;

class Application
{
public:
	Application();
	~Application();

private:
	std::unique_ptr<Window> m_window = nullptr;
	std::unique_ptr<Renderer> m_renderer = nullptr;
};

