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

	Camera& GetCamera() { return camera; }

private:
	Core::WindowParameters m_windowParams;

	Camera camera;
	glm::vec2 mousePos;

	float m_velocity = 1.0f;
	float m_accTime = 1.5f;
	float m_decTime = 0.6f;

	float m_accelerationSensitivity = 0.5f;
	glm::vec3 velocity;
	float m_dampening = 0.85f;

	bool grabMouseFocus = true;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveForward = false;
	bool moveBack = false;
	bool moveUp = false;
	bool moveDown = false;
	bool fastMove = false;
};