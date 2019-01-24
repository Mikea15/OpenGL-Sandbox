#pragma once

#include "Managers/SystemComponentManager.h"

#include "States/DefaultState.h"
#include "States/IBLSpecState.h"
#include "States/InstancingState.h"
#include "States/DeferredRendering.h"
#include "States/SSAOState.h"
#include "States/OcclusionState.h"

class SceneSwitcherSystemComponent
	: public SystemComponent
{
	CLASS_DECLARATION(SceneSwitcherSystemComponent)

public:
	SceneSwitcherSystemComponent();
	~SceneSwitcherSystemComponent();

	void Initialize(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void PreUpdate(float frameTime) override;
	void Update(float deltaTime) override;
	void Render(float alpha) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	Game* m_game;

	std::shared_ptr<State> m_defaultState;
	std::shared_ptr<State> m_instancingState;
	std::shared_ptr<State> m_iblSpecState;
	std::shared_ptr<State> m_deferredRenderingState;
	std::shared_ptr<State> m_ssaoState;
};