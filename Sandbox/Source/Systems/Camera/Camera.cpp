
#include "Camera.h"

#include <iostream>

#include "../Transform.h"


static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock

Camera::Camera()
	: m_position(0.0f, 0.0f, 1.0f)
	, m_positionChange(0.0f)
	, m_viewMatrix(1.0f)
	, m_projectionMatrix(1.0f)
	, m_direction(0.0f)
	, m_up(0.0f)
	, m_right(0.0f)
	, m_horizontalAngle(180.0f)
	, m_verticalAngle(0.0f)
	, m_horizontalChange(0.0f)
	, m_verticalChange(0.0f)
	, m_fov(50.0f)
	, m_nearPlane(0.01f)
	, m_farPlane(150.0f)
	, m_aspectRatio(16.0f / 9.0f)
	, m_movementSpeed(10.0f)
	, m_hasSpeedBoost(false)
	, m_speedBoost(2.0f)
	, m_mouseSensitivity(0.15f, 0.15f)
{
	m_frustum = Frustum(m_viewMatrix);
}

void Camera::ChangeFov(float delta)
{
	SetFov(m_fov + delta);
}

void Camera::SetNearFarPlane(float nearPlane, float farPlane)
{
	assert(nearPlane > 0.0f);
	assert(farPlane > nearPlane);

	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
}

void Camera::Update(float deltaTime)
{
	m_horizontalAngle += m_horizontalChange * m_mouseSensitivity.x;
	m_horizontalChange = 0.0f;

	m_verticalAngle += m_verticalChange * m_mouseSensitivity.y;
	m_verticalChange = 0.0f;

	m_direction = glm::vec3(
		cos(glm::radians(m_verticalAngle)) * sin(glm::radians(m_horizontalAngle)),
		sin(glm::radians(m_verticalAngle)),
		cos(glm::radians(m_verticalAngle)) * cos(glm::radians(m_horizontalAngle))
	);

	m_right = glm::vec3(
		sin(glm::radians(m_horizontalAngle) - 3.14f * 0.5f),
		0.0f,
		cos(glm::radians(m_horizontalAngle) - 3.14f * 0.5f)
	);

	m_up = glm::cross(m_right, m_direction);

	m_position += m_positionChange * (m_hasSpeedBoost ? m_movementSpeed * m_speedBoost : m_movementSpeed) * deltaTime;
	m_positionChange = glm::vec3(0.0f);

	m_viewMatrix = glm::lookAt(m_position, m_position + m_direction, m_up);
	m_projectionMatrix = glm::perspective(
		glm::radians(m_fov),
		m_aspectRatio,
		m_nearPlane,
		m_farPlane
	);

	m_orthographicMatrix = glm::ortho(
		-m_resolution.x * 0.5f, m_resolution.x * 0.5f,
		-m_resolution.y * 0.5f, m_resolution.y * 0.5f, 
		m_nearPlane, 20.0f
	);

	m_frustum.SetViewProjectionMatrix( m_projectionMatrix * m_viewMatrix );
}

void Camera::UpdateFOV(float fovChange)
{

}

void Camera::UpdateMouseMovement(float horizontalChange, float verticalChange)
{
	m_horizontalChange += horizontalChange;
	m_verticalChange += verticalChange;
}

bool Camera::IsVisible(const glm::vec3& position) const
{
	const glm::vec3 camToObj = position - m_position;
	const float distSq = camToObj.length() * camToObj.length();
	if (distSq > m_farPlane * m_farPlane) 
	{ 
		return false; 
	}
	if (distSq < m_nearPlane * m_nearPlane)
	{
		return false;
	}

	const glm::vec3 camToObjDir = glm::normalize(camToObj);
	const float dot = glm::dot(camToObjDir, m_direction);
	if (dot > 0)
	{
		float angle = acosf(dot) * 180 / 3.14f;
		return angle <= m_fov;
	}
	return false;
}

void Camera::Translate(const glm::vec3& offset)
{
	m_positionChange += offset;
}

bool Camera::IsInView(const glm::vec3 & target)
{
	glm::vec3 dir = glm::normalize(target - m_position);
	const float dot = glm::dot(m_direction, dir);
	const float angle = glm::degrees(acosf(dot));

	if (angle < m_fov)
		return true;

	return false;
}

void Camera::LookAt(glm::vec3 position)
{
	assert(position != m_position);
	glm::vec3 direction = glm::normalize(position - m_position);
	m_horizontalAngle = -glm::radians(atan2f(-direction.x, -direction.z));
	m_verticalAngle = glm::radians(asinf(-direction.y));
	normalizeAngles();
}

glm::mat4 Camera::ViewProjectionMatrix()
{
	return m_projectionMatrix * m_viewMatrix;
}

glm::mat4 Camera::ProjectionMatrix()
{
	return m_projectionMatrix;
}

glm::mat4 Camera::View()
{
	return m_viewMatrix;
}

void Camera::normalizeAngles()
{
	m_horizontalAngle = fmodf(m_horizontalAngle, 360.0f);
	//fmodf can return negative values, but this will make them all positive
	if (m_horizontalAngle < 0.0f)
		m_horizontalAngle += 360.0f;

	if (m_verticalAngle > MaxVerticalAngle)
		m_verticalAngle = MaxVerticalAngle;
	else if (m_verticalAngle < -MaxVerticalAngle)
		m_verticalAngle = -MaxVerticalAngle;
}
