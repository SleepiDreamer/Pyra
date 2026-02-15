#pragma once
#include <glfw3.h>
#include <memory>

class Window;
class Renderer;

class Application
{
public:
	Application();
	~Application();

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	std::unique_ptr<Window> m_window = nullptr;
	std::unique_ptr<Renderer> m_renderer = nullptr;
};

