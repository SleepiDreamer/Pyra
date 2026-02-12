#include "Application.h"
#include "Window.h"
#include "Renderer.h"

#include <glfw3.h>

Application::Application()
{
	m_window = std::make_unique<Window>(2560, 1440);
	m_renderer = std::make_unique<Renderer>(*m_window);

	while (!glfwWindowShouldClose(m_window->GetGLFWWindow()))
	{
		glfwPollEvents();
		m_renderer->Render();
	}
}

Application::~Application()
{
	glfwTerminate();
}
