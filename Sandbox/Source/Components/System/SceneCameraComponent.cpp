#include "SceneCameraComponent.h"

#include "Game.h"
#include "Systems/Camera/Camera.h"

CLASS_DEFINITION(SystemComponent, SceneCameraComponent)

SceneCameraComponent::SceneCameraComponent()
	: m_mouseSensitivity(10.0f)
	, m_cameraMovement(0.0f)
{
	m_camera.SetPosition(glm::vec3(0.0f, 1.0f, 7.0f));
	m_mousePosition = glm::vec2(m_windowParams.Width * 0.5f, m_windowParams.Height * 0.5f);
}

SceneCameraComponent::~SceneCameraComponent()
{
}

void SceneCameraComponent::Initialize(Game* game)
{
	m_windowParams = game->GetWindowParameters();
	SDL_SetRelativeMouseMode(m_inputGrabMouse ? SDL_TRUE : SDL_FALSE);
}

void SceneCameraComponent::HandleInput(SDL_Event* event)
{
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F1)
	{
		m_inputGrabMouse = !m_inputGrabMouse;
		SDL_SetRelativeMouseMode(m_inputGrabMouse ? SDL_TRUE : SDL_FALSE);
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_w) m_inputMoveForward = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_s) m_inputMoveBack = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_a) m_inputMoveLeft = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_d) m_inputMoveRight = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e) m_inputMoveUp = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_q) m_inputMoveDown = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_LSHIFT) m_inputEnableMovementBoost = true;

	if (event->type == SDL_KEYDOWN)
	{
		switch (event->key.keysym.sym) {
		case SDLK_f:
			m_camera.LookAt(glm::vec3(0.0f));
			break;
		case SDLK_c: 
			m_camera.ToggleOrthographicCamera();
			break;
		case SDLK_r:
			m_camera.SetPosition(glm::vec3(0.0f));
			break;
		default: break;
		}
	}

	if (event->type == SDL_KEYDOWN &&
		event->key.keysym.sym == SDLK_k) {
		m_camera1 = m_camera.SaveCameraSnapshot();
	}
	if (event->type == SDL_KEYDOWN &&
		event->key.keysym.sym == SDLK_l) {
		m_camera2 = m_camera.SaveCameraSnapshot();
	}

	if (event->type == SDL_KEYDOWN &&
		event->key.keysym.sym == SDLK_i) {
		m_camera.InterpolateTo(m_camera1, m_cameraInterpolationTime);
	}
	if (event->type == SDL_KEYDOWN &&
		event->key.keysym.sym == SDLK_o) {
		m_camera.InterpolateTo(m_camera2, m_cameraInterpolationTime);
	}

	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_w) m_inputMoveForward = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_s) m_inputMoveBack = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_a) m_inputMoveLeft = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_d) m_inputMoveRight = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_e) m_inputMoveUp = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_q) m_inputMoveDown = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_LSHIFT) m_inputEnableMovementBoost = false;


	if (event->type == SDL_MOUSEWHEEL) {
		m_fovInputChange -= event->wheel.y;
	}
}

void SceneCameraComponent::PreUpdate(float frameTime)
{
	
}

void SceneCameraComponent::Update(float deltaTime)
{
	int x, y;
	SDL_GetRelativeMouseState(&x, &y);
	m_mousePosition = glm::vec2(x, y);

	// get mouse input
	glm::vec2 mouseMovement = m_mousePosition * m_mouseSensitivity * deltaTime;
	
	// get camera movement input
	glm::vec3 direction = m_camera.GetForward() * ((m_inputMoveForward) ? 1.0f : (m_inputMoveBack) ? -1.0f : 0.0f);
	direction += m_camera.GetRight() * ((m_inputMoveRight) ? 1.0f : (m_inputMoveLeft) ? -1.0f : 0.0f);
	direction += m_camera.GetUp() * ((m_inputMoveUp) ? 1.0f : (m_inputMoveDown) ? -1.0f : 0.0f);
	
	const float velocity = m_cameraVelocity * (m_inputEnableMovementBoost ? m_cameraMovementSpeedBoostMult : 1.0f);
	m_cameraMovement += direction * velocity * m_cameraSensitivity * deltaTime;
	
	// only apply damping if not accelerating
	if (glm::length(direction) < 0.01f) 
	{
		m_cameraMovement *= m_cameraMovementDamping;
	}

	if (m_fovInputChange != 0)
	{
		m_fovChange = m_fovInputChange *  m_fovVelocity * m_fovSensitivity * deltaTime;
		m_fovInputChange = 0;
	}
	m_fovChange *= m_fovDamping;

	if (m_inputGrabMouse)
	{
		m_camera.UpdateLookAt(mouseMovement);
	}
	m_camera.Move(m_cameraMovement);
	m_camera.UpdateFov(m_fovChange);
	m_camera.Update(deltaTime);
}

void SceneCameraComponent::Render(float alpha)
{
	
}

void SceneCameraComponent::RenderUI()
{
	ImGui::Begin("Camera Interpolation");
	ImGui::SliderFloat("Interpolation Time", &m_cameraInterpolationTime, 0.0f, 100.0f);
	if (ImGui::Button("Camera 1"))
	{
		m_camera.InterpolateTo(m_camera1, m_cameraInterpolationTime);
	}
	ImGui::SameLine();
	if (ImGui::Button("Camera 2"))
	{
		m_camera.InterpolateTo(m_camera2, m_cameraInterpolationTime);
	}
	ImGui::End();

	ImGui::Begin("Scene Camera");

	Camera::Params currentParams = m_camera.GetParams();

	ImGui::Text("Camera Speed");
	bool changedSpeedViaButon = false;
	if (ImGui::Button("1.0")) { m_cameraVelocity = 1.0f; } ImGui::SameLine();
	if (ImGui::Button("10.0")) { m_cameraVelocity = 10.0f; } ImGui::SameLine();
	if (ImGui::Button("100.0")) { m_cameraVelocity = 100.0f; }
	
	ImGui::SliderFloat("Camera Velocity", &m_cameraVelocity, 0.0f, 100.0f);
	ImGui::SliderFloat("Camera Sensitivity", &m_cameraSensitivity, 0.0f, 1.0f);
	ImGui::SliderFloat("Camera Damping", &m_cameraMovementDamping, 0.0f, 1.0f);
	
	ImGui::SliderFloat("Fov Velocity", &m_fovVelocity, 0.0f, 100.0f);
	ImGui::SliderFloat("Fov Sensitivity", &m_fovSensitivity, 0.0f, 1.0f);
	ImGui::SliderFloat("Fov Damping", &m_fovDamping, 0.0f, 1.0f);

	ImGui::SliderFloat("Near Plane", &currentParams.m_nearPlane, 0.01f, currentParams.m_farPlane);
	ImGui::SliderFloat("Far Plane", &currentParams.m_farPlane, currentParams.m_nearPlane, 1000.0f);
	ImGui::SliderFloat("FoV", &currentParams.m_fov, 0.1f, 179.0f);
	
	m_camera.SetParams(currentParams);

	ImGui::End();
}

void SceneCameraComponent::Cleanup()
{

}
