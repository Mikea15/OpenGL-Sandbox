#pragma once

#include "Managers/SystemComponentManager.h"

#include "Systems/Camera/Camera.h"

struct Core::WindowParameters;

class SceneCameraComponent
	: public SystemComponent
{
	CLASS_DECLARATION(SceneCameraComponent)

public:
	SceneCameraComponent();
	~SceneCameraComponent();

	void Initialize(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void PreUpdate(float frameTime) override;
	void Update(float deltaTime) override;
	void Render(float alpha) override;
	void RenderUI() override;
	void Cleanup() override;

	Camera& GetCamera() { return m_camera; }

private:
	Core::WindowParameters m_windowParams;

	Camera m_camera;

	glm::vec3 m_cameraMovement;
	float m_cameraVelocity = 10.0f;
	float m_cameraSensitivity = 1.0f;
	float m_cameraMovementDamping = 0.85f;
	float m_cameraMovementSpeedBoostMult = 5.0f;

	glm::vec2 m_mousePosition;
	glm::vec2 m_mouseSensitivity;

	int m_fovInputChange = 0;
	float m_fovChange = 0.0f;
	float m_fovVelocity = 100.0f;
	float m_fovSensitivity = 1.0f;
	float m_fovDamping = 0.85f;

	bool m_inputGrabMouse = true;
	bool m_inputMoveLeft = false;
	bool m_inputMoveRight = false;
	bool m_inputMoveForward = false;
	bool m_inputMoveBack = false;
	bool m_inputMoveUp = false;
	bool m_inputMoveDown = false;
	bool m_inputEnableMovementBoost = false;
	bool m_inputToggleOrthoCamera = false;
};