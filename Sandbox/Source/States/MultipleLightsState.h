
#pragma once

#include <SDL.h>
#include <gl/glew.h>

#include "Dependencies/std_image/stb_image.h"

#include "State.h"

#include "Systems/Camera/Camera.h"
#include "Systems/Rendering/Skybox.h"
#include "Managers/ShaderManager.h"

#include "Components/System/SceneCameraComponent.h"
#include "Systems/Entity.h"


class MultipleLightsState
	: public State
{
public:
	MultipleLightsState();
	~MultipleLightsState() override {};

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void Cleanup() override;

private:
	const Core::SDLHandler* m_sdlHandler;
	Core::WindowParameters m_windowParams;
	SceneCameraComponent* m_sceneCameraComp;

	ShaderManager m_shaders;

	Entity cubeObject;

	float rotationAngle = 0.0f;
	float rotationPerSecond = 90.0f;

	// vertex array object
	// vertex 
	unsigned int VBO, cubeVAO, lightVAO, EBO;
	unsigned int diffuseMap, specularMap;
	Shader lightingShader;
	Shader lampShader;

	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> pointLightPositions;

	glm::vec3 lightPos;

	Skybox m_skybox;
	Shader skyboxShader;
};
