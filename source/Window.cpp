#include "Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#include <glfw3native.h>
#include <assert.h>
#include <iostream>

Window::Window(const int width, const int height, const char* title)
{
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}

	m_monitor = glfwGetPrimaryMonitor();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	m_hwnd = glfwGetWin32Window(m_window);

	m_width = width;
	m_height = height;
}
