#pragma once

#include "DefaultState.h"

#include "Systems/PostProcessing/SSAO.h"

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

	bool enableSSAO = true;

	Shader shaderGeometryPass;
	Shader shaderLightingPass;
	Shader shaderLightBox;
	Shader shaderSSAO;
	Shader shaderSSAOBlur;
	
	Entity ent;
	std::vector<glm::vec3> objectPositions;

	Quad quad;
	Cube cube;

	SSAO ssaoFx;
};

