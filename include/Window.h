#pragma once
#include <windows.h>
#include <memory>

struct GLFWwindow;
struct GLFWmonitor;

class Window
{
public:
	Window(int width, int height, const char* title = "Kyra");
	~Window() = default;

	[[nodiscard]] GLFWwindow* GetGLFWWindow() const { return m_window; }
	[[nodiscard]] HWND GetHWND() const { return m_hwnd; }
	[[nodiscard]] int GetWidth() const { return m_width; }
	[[nodiscard]] int GetHeight() const { return m_height; }

private:
	HWND m_hwnd = nullptr;
	GLFWwindow* m_window;
	GLFWmonitor* m_monitor;
	int m_width = -1;
	int m_height = -1;
};

