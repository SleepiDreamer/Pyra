#include "Application.h"
#include "Window.h"
#include "Renderer.h"

#include <glfw3.h>

Application::Application()
{
	m_window = std::make_unique<Window>(2560, 1440);
	m_renderer = std::make_unique<Renderer>(*m_window);

	auto glfwWindow = m_window->GetGLFWWindow();

	glfwSetKeyCallback(glfwWindow, KeyCallback);
	glfwSetWindowUserPointer(glfwWindow, this);

	while (!glfwWindowShouldClose(glfwWindow))
	{
		glfwPollEvents();

		m_renderer->Render();
	}
}

Application::~Application()
{
	glfwTerminate();
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if ((key == GLFW_KEY_ENTER && action == GLFW_PRESS) && (mods & GLFW_MOD_ALT))
	{
		app->m_renderer->ToggleFullscreen();
	}
}