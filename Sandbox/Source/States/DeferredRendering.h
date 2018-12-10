#pragma once

#include <random>

#include "DefaultState.h"

#include "Systems/Rendering/Primitives/Quad.h"
#include "Systems/Rendering/Primitives/Cube.h"

class DeferredRendering
	: public DefaultState
{
public:
	DeferredRendering();
	~DeferredRendering() override;

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec;
	unsigned int rboDepth;

	Shader shaderGeometryPass;
	Shader shaderLightingPass;
	Shader shaderLightBox;
	Shader shaderSSAO;
	Shader shaderSSAOBlur;

	Quad quad;
	Entity ent;
	std::vector<glm::vec3> objectPositions;

	const unsigned int nLights = 32;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;

	float lConstant = 1.0f;
	float llinear = 0.3f;
	float lquadratic = 0.8f;
};

