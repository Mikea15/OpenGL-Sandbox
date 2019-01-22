
#include "Camera.h"

#include <iostream>

#include "../Transform.h"


//must be less than 90 to avoid gimbal lock
const float Camera::s_maxVerticalAngle = 85.0f;
const float Camera::s_minFov = 0.1f;
const float Camera::s_maxFov = 179.0f;

Camera::Camera()
	: m_position(0.0f, 0.0f, 1.0f)
	, m_viewMatrix(1.0f)
	, m_projectionMatrix(1.0f)
	, m_forward(0.0f)
	, m_up(0.0f)
	, m_right(0.0f)
	, m_horizontalAngle(180.0f)
	, m_verticalAngle(0.0f)
{
	m_frustum = BoundingFrustum(m_viewMatrix);
}

void Camera::Update(float deltaTime)
{
	m_forward = glm::vec3(
		cos(glm::radians(m_verticalAngle)) * sin(glm::radians(m_horizontalAngle)),
		sin(glm::radians(m_verticalAngle)),
		cos(glm::radians(m_verticalAngle)) * cos(glm::radians(m_horizontalAngle))
	);

	m_right = glm::vec3(
		sin(glm::radians(m_horizontalAngle) - 3.14f * 0.5f),
		0.0f,
		cos(glm::radians(m_horizontalAngle) - 3.14f * 0.5f)
	);

	m_up = glm::cross(m_right, m_forward);

	const float hSize = m_params.m_orthoSize * 0.5f;
	const float wSize = m_params.m_orthoSize * 0.5f * m_params.m_aspectRatio;

	m_viewMatrix = glm::lookAt(m_position, m_position + m_forward, m_up);
	m_projectionMatrix = glm::perspective( glm::radians(m_params.m_fov), m_params.m_aspectRatio,
		m_params.m_nearPlane, m_params.m_farPlane
	);
	m_orthographicMatrix = glm::ortho(-wSize, wSize, -hSize, hSize, 
		m_params.m_nearPlane, m_params.m_farPlane);

	m_frustum.UpdateViewProjectionMatrix(m_projectionMatrix * m_viewMatrix);
}

void Camera::UpdateFov(float delta)
{
	m_params.m_fov += delta;
	if (m_params.m_fov < s_minFov)
	{
		m_params.m_fov = s_minFov;
	}
	else if (m_params.m_fov > s_maxFov)
	{
		m_params.m_fov = s_maxFov;
	}
}

void Camera::UpdateLookAt(const glm::vec2& mouseMovement)
{
	m_horizontalAngle -= mouseMovement.x;
	m_verticalAngle -= mouseMovement.y;
	NormalizeAngles();
}

void Camera::Move(const glm::vec3& movement)
{
	m_position += movement;
}

void Camera::SetFov(float fov)
{
	if (fov < s_minFov)
	{
		fov = s_minFov;
	}
	else if (fov > s_maxFov)
	{
		fov = s_maxFov;
	}
	m_params.m_fov = fov;
}

void Camera::SetNearFarPlane(float nearPlane, float farPlane)
{
	//assert(nearPlane > 0.0f);
	//assert(farPlane > nearPlane);

	m_params.m_nearPlane = nearPlane;
	m_params.m_farPlane = farPlane;
}

void Camera::SetAspectRatio(float ratio)
{
	//assert(ratio > 0.0);
	m_params.m_aspectRatio = ratio;
}

bool Camera::IsInFieldOfView(const glm::vec3& position) const
{
	const glm::vec3 camToObj = position - m_position;
	const float distSq = camToObj.length() * camToObj.length();
	if (distSq > m_params.m_farPlane * m_params.m_farPlane)
	{
		return false;
	}
	if (distSq < m_params.m_nearPlane * m_params.m_nearPlane)
	{
		return false;
	}

	const glm::vec3 camToObjDir = glm::normalize(camToObj);
	const float dot = glm::dot(camToObjDir, m_forward);
	if (dot > 0)
	{
		const float angle = glm::degrees(acosf(dot));
		return angle <= m_params.m_fov;
	}
	return false;
}

void Camera::ToggleOrthographicCamera()
{
	m_params.m_isOrtho = !m_params.m_isOrtho;
}

void Camera::LookAt(const glm::vec3& position)
{
	assert(position != m_position);
	glm::vec3 direction = glm::normalize(position - m_position);
	m_horizontalAngle = -glm::radians(atan2f(-direction.x, -direction.z));
	m_verticalAngle = glm::radians(asinf(-direction.y));
	NormalizeAngles();
}

void Camera::NormalizeAngles()
{
	while (m_horizontalAngle < 0.0f)
	{
		m_horizontalAngle += 360.0f;
	}

	while (m_horizontalAngle > 360.0f)
	{
		m_horizontalAngle -= 360.0f;
	}

	if (m_verticalAngle > s_maxVerticalAngle)
	{
		m_verticalAngle = s_maxVerticalAngle;
	}
	else if (m_verticalAngle < -s_maxVerticalAngle)
	{
		m_verticalAngle = -s_maxVerticalAngle;
	}
}

void Camera::SetParams(const CameraParams & params)
{
	if (m_params != params)
	{
		m_params = params;
	}
}
