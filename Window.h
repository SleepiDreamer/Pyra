#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <memory>

struct GLFWwindow;
struct GLFWmonitor;

class Window
{
public:
	Window(int width, int height);
	~Window() = default;
	
	GLFWwindow* GetGLFWWindow() const { return m_window; }

private:
	HWND m_hwnd = nullptr;
	GLFWwindow* m_window;
	GLFWmonitor* m_monitor;
	int m_width = -1;
	int m_height = -1;
};

