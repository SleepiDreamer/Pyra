#include "Camera.h"

void Camera::SetDirection(const glm::vec3& forward)
{
	m_forward = glm::normalize(forward);
	m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	m_up = glm::normalize(glm::cross(m_right, m_forward));
}

void Camera::Move(const glm::vec3& delta)
{
	m_position += delta;
}

void Camera::Rotate(const glm::vec2& delta)
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), delta.x, m_up) * glm::rotate(glm::mat4(1.0f), delta.y, m_right);
	m_forward = glm::normalize(glm::vec3(rotation * glm::vec4(m_forward, 0.0f)));
	m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	m_up = glm::normalize(glm::cross(m_right, m_forward));
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_position, m_position + m_forward, m_up);
}