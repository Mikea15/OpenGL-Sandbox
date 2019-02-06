#pragma once

struct CameraParams
{
	float m_fov = 70.0f;
	float m_nearPlane = 0.01f;
	float m_farPlane = 350.0f;
	float m_aspectRatio = 16.0f / 9.0f;
	float m_orthoSize = 5.0f;

	bool m_isOrtho = false;

	bool operator==(const CameraParams& rhs) {
		if (m_fov != rhs.m_fov) return false;
		if (m_nearPlane != rhs.m_nearPlane) return false;
		if (m_farPlane != rhs.m_farPlane) return false;
		if (m_aspectRatio != rhs.m_aspectRatio) return false;
		if (m_orthoSize != rhs.m_orthoSize) return false;
		if (m_isOrtho != rhs.m_isOrtho) return false;
		return true;
	}

	bool operator!=(const CameraParams& rhs) {
		return !(*this == rhs);
	}
};