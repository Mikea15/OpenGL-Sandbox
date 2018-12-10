#pragma once

#include <random>

#include "DefaultState.h"

#include "Systems/Rendering/Primitives/Quad.h"
#include "Systems/Rendering/Primitives/Cube.h"

class SSAOState
	: public DefaultState
{
public:
	SSAOState();
	~SSAOState() override;

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedo;
	unsigned int renderBufferObjectDepth;

	Shader shaderGeometryPass;
	Shader shaderLightingPass;
	Shader shaderLightBox;
	Shader shaderSSAO;
	Shader shaderSSAOBlur;

	Entity ent;
	std::vector<glm::vec3> objectPositions;

	Quad quad;
	Cube cube;

	// ssao
	std::vector<glm::vec3> ssaoKernel;
	unsigned int noiseTexture;
	unsigned int ssaoFBO;
	unsigned int ssaoBlurFBO;
	unsigned int ssaoColorBuffer;
	unsigned int ssaoColorBufferBlur;

	bool enableSSAO = true;
	float ssaoIntensity = 1.0f;
	int kernelSize = 64;
	float radius = 0.5f;
	float bias = 0.025f;
};

