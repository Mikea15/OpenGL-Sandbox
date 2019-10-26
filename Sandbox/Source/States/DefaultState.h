#pragma once

#include <SDL.h>
#include <GL/glew.h>

#include "nlohmann/json.hpp"

#include "State.h"

#include "Core/SDLHandler.h"

#include "Managers/ShaderManager.h"
#include "Managers/SceneManager.h"

#include "Systems/Entity.h"
#include "Systems/Camera/Camera.h"
#include "Systems/Rendering/Skybox.h"
#include "Systems/Rendering/Primitives.h"
#include "Systems/Rendering/Primitives/Quad.h"
#include "Systems/Rendering/Terrain.h"

#include "Components/System/SceneCameraComponent.h"

using namespace nlohmann;

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
	SDLHandler* m_sdlHandler;
	WindowParams m_windowParams;
	WindowParams currentParams;

	SceneCameraComponent* m_sceneCamera;
	AssetManager* m_assetManager;

	ShaderManager shaderManager;
	SceneManager m_sceneManager;
	
	Shader skyboxShader;
	Skybox m_skybox;
	Skybox::Settings skyboxSettings;

	std::shared_ptr<Model> m_model;
	Shader m_simpleShader;
};