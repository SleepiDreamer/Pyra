#include "Application.h"
#include "Window.h"
#include "Renderer.h"
#include "Camera.h"

#define GLFW_INCLUDE_NONE
#include <glfw3.h>

Application::Application()
{
	m_window = std::make_unique<Window>(1920, 1080);
	m_renderer = std::make_unique<Renderer>(*m_window);
	m_camera = std::make_shared<Camera>();
	m_renderer->SetCamera(m_camera);

	auto glfwWindow = m_window->GetGLFWWindow();

	glfwSetKeyCallback(glfwWindow, KeyCallback);
	glfwSetWindowUserPointer(glfwWindow, this);

	float deltaTime = 0.0f;
	static float lastFrameTime = 0.0f;
	while (!glfwWindowShouldClose(glfwWindow))
	{
		float currentFrameTime = static_cast<float>(glfwGetTime());
		deltaTime = currentFrameTime - lastFrameTime;

		glfwPollEvents();

		Update(deltaTime);

		m_renderer->Render();

		lastFrameTime = currentFrameTime;
	}
}

Application::~Application()
{
	glfwTerminate();
}

void Application::Update(const float deltaTime)
{
	float cameraSpeed = 1.0f;
	if (glfwGetKey(m_window->GetGLFWWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		cameraSpeed *= 4.0f;
	}
	if (glfwGetKey(m_window->GetGLFWWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		cameraSpeed *= 0.25f;
	}

	auto window = m_window->GetGLFWWindow();
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // W
	{
		m_camera->Move(deltaTime * m_camera->GetForward());
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // S
	{
		m_camera->Move(-deltaTime * m_camera->GetForward());
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // A
	{
		m_camera->Move(-deltaTime * m_camera->GetRight());
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // D
	{
		m_camera->Move(deltaTime * m_camera->GetRight());
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // Q
	{
		m_camera->Move(glm::vec3(0, -deltaTime, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // R
	{
		m_camera->Move(glm::vec3(0, deltaTime, 0));
	}
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if ((key == GLFW_KEY_ENTER && action == GLFW_PRESS) && (mods & GLFW_MOD_ALT))
	{
		app->m_renderer->ToggleFullscreen();
	}
}