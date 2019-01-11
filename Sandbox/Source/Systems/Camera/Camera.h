#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

#include "../Geometry/BoundingFrustum.h"


class Camera
{
public:
	Camera();
	~Camera() = default;

	const glm::vec3& GetPosition() { return m_position; }
	void SetPosition(const glm::vec3& position) { m_position = position; }

	float GetFov() { return m_fov; }
	void SetFov(float fov)
	{
		if (fov < 0.0f)
			fov = 0.1f;
		else if (fov > 180.0f)
			fov = 180.0f;
		
		m_fov = fov; 
	}

	void ChangeFov(float delta);

	float GetNearPlane() { return m_nearPlane; }
	float GetFarPlane() { return m_farPlane; }
	void SetNearFarPlane(float nearPlane, float farPlane);

	float GetAspectRatio() { return m_aspectRatio; }
	void SetAspectRatio(float ratio) 
	{
		assert(ratio > 0.0);
		m_aspectRatio = ratio; 
	}

	bool HasSpeedBoost() const { return m_hasSpeedBoost; }
	void SetSpeedBoost(bool enable) { m_hasSpeedBoost = enable; }
	void SetCameraSpeed(float speed) { m_movementSpeed = speed; }

	void Update(float deltaTime);
	void UpdateFOV(float fovChange);
	void UpdateMouseMovement(float horizontalChange, float verticalChange);

	bool IsVisible(const glm::vec3& position) const;

	glm::mat4 GetViewMatrix() { return m_viewMatrix; }

	void Translate(const glm::vec3& offset);

	bool IsInView(const glm::vec3& target);

	
	/**
	 Orients the camera so that is it directly facing `position`
	 @param position  the position to look at
	 */
	void LookAt(glm::vec3 position);
		
	glm::mat4 ViewProjectionMatrix();
	glm::mat4 ProjectionMatrix();
	const glm::mat4& OrthographicMatrix() const { return m_orthographicMatrix; }
	glm::mat4 View();

	void normalizeAngles();

	glm::vec3 m_position;
	glm::vec3 m_positionChange;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_orthographicMatrix;

	glm::vec3 m_direction;
	glm::vec3 m_up;
	glm::vec3 m_right;

	glm::vec2 m_resolution;

	float m_horizontalAngle;
	float m_verticalAngle;
	float m_horizontalChange;
	float m_verticalChange;
	float m_fov;
	float m_nearPlane;
	float m_farPlane;
	float m_aspectRatio;

	// Camera options
	float m_movementSpeed;
	bool m_hasSpeedBoost;
	float m_speedBoost;
	glm::vec2 m_mouseSensitivity;

	BoundingFrustum m_frustum;
};

