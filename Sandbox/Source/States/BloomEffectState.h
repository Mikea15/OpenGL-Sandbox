#pragma once

#include <SDL.h>
#include <GL/glew.h>

#include "State.h"

#include "Core/SDLHandler.h"

#include "Managers/ShaderManager.h"
#include "Managers/AssetManager.h"

#include "Systems/Entity.h"
#include "Systems/Camera/Camera.h"
#include "Systems/Rendering/Skybox.h"
#include "Systems/Rendering/Primitives/Quad.h"

#include "Components/System/SceneCameraComponent.h"

class BloomEffectState
	: public State
{
public:
	BloomEffectState() = default;
	~BloomEffectState() = default;

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

	AssetManager assetManager;
	// shaders
	ShaderManager shaderManager;
	Shader shader;
	Shader shaderLight;
	Shader shaderBlur;
	Shader shaderBloomFinal;
	Shader skyboxShader;

	unsigned int woodTexture;
	unsigned int containerTexture;

	unsigned int hdrFBO;
	unsigned int colorBuffers[2];
	unsigned int rboDepth;

	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];

	Skybox skybox;

	Quad quad;

	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;

	Entity cube;

	Transform trans[3];

	bool hdr = false;
	bool bloom = false;
	float exposure = 1.0f;
	int blurPasses = 10;
	bool useBetterGauss = false;
	float blurRadius = 2.0f;
	bool renderBlurPass = false;
	float renderScale = 1.0f;

	glm::vec3 topColor = glm::vec3(0.22, 0.351, 0.491);
	float topExp = 50.0f;
	glm::vec3 horizonColor = glm::vec3(1, 1, 1);

	glm::vec3 bottomColor = glm::vec3(0.475, 0.316, 0.228);
	float bottomExp = 50.0f;

	float skyIntensity = 1.10f;

	glm::vec3 sunColor = glm::vec3(0.895, 0.93, 0.526);
	float sunIntensity = 1.50f;

	float sunAlpha = 550.0f;
	float sunBeta = 1.0f;

	float sunAzimuth = 175.0f;
	float sunAltitude = 25.0f;

};