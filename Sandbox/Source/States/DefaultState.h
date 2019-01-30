#pragma once

#include <SDL.h>
#include <GL/glew.h>

#include "State.h"

#include "Core/SDLHandler.h"

#include "Managers/ShaderManager.h"

#include "Systems/Entity.h"
#include "Systems/Camera/Camera.h"
#include "Systems/Rendering/Skybox.h"
#include "Systems/Rendering/Primitives.h"
#include "Systems/Rendering/Primitives/Quad.h"

#include "Components/System/SceneCameraComponent.h"



class DefaultState
	: public State
{
public:
	DefaultState() = default;
	virtual ~DefaultState() = default;

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void RenderUI() override;
	void Cleanup() override;

protected:
	Core::SDLHandler* m_sdlHandler;
	Core::WindowParameters m_windowParams;
	Core::WindowParameters currentParams;

	SceneCameraComponent* m_sceneCameraComp;
	AssetManager* m_assetManager;

	ShaderManager shaderManager;
	Shader skyboxShader;

	Skybox m_skybox;
	SkyboxSettings skyboxSettings;
};