
#pragma once

#include <gl/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Dependencies/imgui/imgui.h"

#include "State.h"
#include "Core/SDLHandler.h"

#include "Systems/Model.h"
#include "Systems/Camera/Camera.h"

#include "Managers/ShaderManager.h"
#include "Components/System/SceneCameraComponent.h"

class AdvancedLighting
	: public State
{
public:
	AdvancedLighting();
	~AdvancedLighting() override {};

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	const Core::SDLHandler* m_sdlHandler;
	Core::WindowParameters m_windowParams;
	SceneCameraComponent* m_sceneCameraComp;

	unsigned int gridVAO, gridVBO;
	Shader gridShader;

	unsigned int cubeVAO, cubeVBO;
	unsigned int planeVAO, planeVBO;
	unsigned int floorTexture;
	unsigned int floorTextureGammaCorrected;

	unsigned int xVAO, xVBO;
	unsigned int yVAO, yVBO;
	unsigned int zVAO, zVBO;

	unsigned int depthMapFBO;
	unsigned int depthMap;

	ShaderManager m_shaders;

	Shader shader;
	Shader blue, green, red;
	Shader modelShader;
	Shader depthShader;
	Model cube1M;
	Model plane10M;

	glm::vec3 lightPosition;
	std::vector<glm::vec3> lightPos;
	std::vector<glm::vec3> lightColors;

	bool isBlinn = false;
	bool isGammaCorrected = false;
}; 
