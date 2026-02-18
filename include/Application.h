#pragma once
#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include <memory>

class Window;
class Camera;
class Renderer;

class Application
{
public:
	Application();
	~Application();

	void Update(float deltaTime);

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	std::unique_ptr<Window> m_window = nullptr;
	std::shared_ptr<Camera> m_camera = nullptr;
	std::unique_ptr<Renderer> m_renderer = nullptr;

	double m_mouseXPrev = 0.0f;
	double m_mouseYPrev = 0.0f;
};

