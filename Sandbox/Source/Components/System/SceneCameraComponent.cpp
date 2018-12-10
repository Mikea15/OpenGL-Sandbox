#include "SceneCameraComponent.h"

#include "Game.h"
#include "Systems/Camera/Camera.h"

CLASS_DEFINITION(SystemComponent, SceneCameraComponent)

SceneCameraComponent::SceneCameraComponent()
{
	camera = Camera();
	camera.SetPosition(glm::vec3(0.0f, 1.0f, 7.0f));
	camera.SetNearFarPlane(0.001f, 500.0f);
	camera.SetFov(75.0f);

	mousePos = glm::vec2(m_windowParams.Width * 0.5f, m_windowParams.Height * 0.5f);

	velocity = glm::vec3(0.0f);
}

SceneCameraComponent::~SceneCameraComponent()
{
}

void SceneCameraComponent::Initialize(Game* game)
{
	m_windowParams = game->GetWindowParameters();

	// grab mouse focus
	SDL_SetRelativeMouseMode(grabMouseFocus ? SDL_TRUE : SDL_FALSE);
}

void SceneCameraComponent::HandleInput(SDL_Event* event)
{
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F1)
	{
		grabMouseFocus = !grabMouseFocus;
		SDL_SetRelativeMouseMode(grabMouseFocus ? SDL_TRUE : SDL_FALSE);
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_w) moveForward = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_s) moveBack = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_a) moveLeft = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_d) moveRight = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e) moveUp = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_q) moveDown = true;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_LSHIFT) fastMove = true;

	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_w) moveForward = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_s) moveBack = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_a) moveLeft = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_d) moveRight = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_e) moveUp = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_q) moveDown = false;
	if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_LSHIFT) fastMove = false;

	if (event->type == SDL_MOUSEWHEEL) {
		float delta = event->wheel.y * 2.0f;
		camera.ChangeFov(delta);
	}
}

void SceneCameraComponent::PreUpdate(float frameTime)
{
	
}

void SceneCameraComponent::Update(float deltaTime)
{
	int x, y;
	SDL_GetRelativeMouseState(&x, &y);

	mousePos = glm::vec2(x, y);

	if (grabMouseFocus)
	{
		camera.UpdateMouseMovement(static_cast<float>(-x), static_cast<float>(-y));
	}

	camera.Update(deltaTime);
	camera.SetSpeedBoost(fastMove);

	glm::vec3 direction = camera.m_direction * ((moveForward) ? 1.0f : (moveBack) ? -1.0f : 0.0f);
	direction += camera.m_right * ((moveRight) ? 1.0f : (moveLeft) ? -1.0f : 0.0f);
	direction += camera.m_up * ((moveUp) ? 1.0f : (moveDown) ? -1.0f : 0.0f);
	
	velocity += direction * m_velocity * m_accelerationSensitivity;
	velocity *= m_dampening;

	camera.Translate(velocity);
}

void SceneCameraComponent::Render(float alpha)
{
	
}

void SceneCameraComponent::RenderUI()
{
	ImGui::Begin("Scene Camera");
	ImGui::Text("Current Fov: %f", camera.m_fov);
	ImGui::Separator();
	ImGui::Text("Camera Speed");
	bool changedSpeedViaButon = false;
	if (ImGui::Button("1.0")) { m_velocity = 1.0f; } ImGui::SameLine();
	if (ImGui::Button("10.0")) { m_velocity = 10.0f; } ImGui::SameLine();
	if (ImGui::Button("100.0")) { m_velocity = 100.0f; }
	
	if (changedSpeedViaButon)
	{
		camera.SetCameraSpeed(m_velocity);
	}

	ImGui::SliderFloat("Camera Velocity", &m_velocity, 0.0f, 100.0f);
	ImGui::SliderFloat("Camera Acc Sensitivity", &m_accelerationSensitivity, 0.0f, 1.0f);
	ImGui::SliderFloat("Camera Speed Damp", &m_dampening, 0.0f, 1.0f);
	ImGui::SliderFloat("Near Plane", &camera.m_nearPlane, 0.01f, camera.m_farPlane);
	ImGui::SliderFloat("Far Plane", &camera.m_farPlane, camera.m_nearPlane, 1000.0f);
	ImGui::SliderFloat("FoV", &camera.m_fov, 0.1f, 180.0f);
	ImGui::End();
}

void SceneCameraComponent::Cleanup()
{

}
