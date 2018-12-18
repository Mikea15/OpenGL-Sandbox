
#pragma once

#include <SDL.h>
#include <gl/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Core/SDLHandler.h"

#include "State.h"
#include "Systems/Model.h"
#include "Systems/Transform.h"
#include "Systems/Camera/Camera.h"
#include "Systems/Rendering/Primitives/Quad.h"

#include "Systems/Entity.h"
#include "Systems/Rendering/Skybox.h"
#include "Systems/Rendering/Terrain.h"
#include "Systems/Rendering/Primitives/Quad.h"
#include "Systems/Rendering/TextureBuffer.h"

#include "Managers/ShaderManager.h"

#include "Components/System/SceneCameraComponent.h"

#include "Core/Utils.h"

class ShadowMappingState
	: public State
{
public:
	ShadowMappingState();
	~ShadowMappingState() override {};

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
	
	float deltaTime;
	float totalTime;

	ShaderManager m_shaders;
	Shader skyboxShader;
	Shader shader;
	Shader simpleDepthShader;
	Shader reflectionShader;
	Shader refractionShader;
	Shader hdrShader;
	Shader multipleLightsShader;

	Entity cubeObject;
	Entity sphereObject;
	Entity plane;
	Entity metalSphere;
	Entity refractSphere;
	float m_refractionIndex = 1.33f;

	
	glm::vec2 m_planeSize;
	bool m_planeSizeChanged = false;
	Terrain m_terrain;
	Transform planeTransform;

	Skybox m_skybox;

	bool shadows = false;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	unsigned int woodTexture;
	unsigned int uvTexture;
	unsigned int depthCubemap;
	glm::vec3 lightPos;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;


	bool isHdrEnabled = false;
	float exposure = 1.0f;

	unsigned int colorBuffer;
	unsigned int hrdFrameBufferObject;
	unsigned int rboDepth;

	unsigned int depthFBO;
	unsigned int depthMap;
}; 
