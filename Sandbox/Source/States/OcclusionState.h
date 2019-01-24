#pragma once

#include "DefaultState.h"

#include "Systems/QuadTree.h"
#include "Systems/Octree.h"


class OcclusionState
	: public DefaultState
{
public:
	OcclusionState() = default;
	virtual ~OcclusionState() = default;

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void RenderUI() override;
	void Cleanup() override;

protected:
	QuadTree m_qTree;
	Octree m_oTree;

	Shader wireframeShader;
	Transform scratchTransform;

	std::vector<glm::vec3> positions;
	std::vector<ContainmentType> visible;

	Camera cc;
};
