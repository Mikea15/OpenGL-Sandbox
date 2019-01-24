#include "SceneSwitcherSystemComponent.h"

#include "Game.h"

CLASS_DEFINITION(SystemComponent, SceneSwitcherSystemComponent)

SceneSwitcherSystemComponent::SceneSwitcherSystemComponent()
{
	
}

SceneSwitcherSystemComponent::~SceneSwitcherSystemComponent()
{

}

void SceneSwitcherSystemComponent::Initialize(Game* game)
{
	m_game = game;

	// PBR Shader requires that the textures be passed at layout = 3 instead of 0. ( or not? )
	// still. I'll leave this comment here. I spent too much time debuging this shit.
	m_defaultState = std::make_shared<IBLSpecState>(); 
	// m_defaultState = std::make_shared<OcclusionState>(); 
	// m_defaultState = std::make_shared<DeferredRendering>();
	// m_defaultState = std::make_shared<DefaultState>();
	m_game->ChangeState(m_defaultState);
}

void SceneSwitcherSystemComponent::HandleInput(SDL_Event* event)
{
	// disable state/scene change
	return;

	if (event->type == SDL_KEYDOWN)
	{
		switch (event->key.keysym.sym)
		{
		case SDLK_1:
			m_game->ChangeState(m_defaultState);
			break;
		case SDLK_2:
			if (m_iblSpecState == nullptr)
			{
				m_iblSpecState = std::make_shared<IBLSpecState>();
			}
			m_game->ChangeState(m_iblSpecState);
			break;
		case SDLK_3:
			if (m_instancingState == nullptr)
			{
				m_instancingState = std::make_shared<InstancingState>();
			}
			m_game->ChangeState(m_instancingState);
			break;
		case SDLK_4:
			if (m_deferredRenderingState == nullptr)
			{
				m_deferredRenderingState = std::make_shared<DeferredRendering>();
			}
			m_game->ChangeState(m_deferredRenderingState);
			break;
		case SDLK_5:
			if (m_ssaoState == nullptr)
			{
				m_ssaoState = std::make_shared<SSAOState>();
			}
			m_game->ChangeState(m_ssaoState);
			break;
		}
	}
}

void SceneSwitcherSystemComponent::PreUpdate(float frameTime)
{

}

void SceneSwitcherSystemComponent::Update(float deltaTime)
{
	
}

void SceneSwitcherSystemComponent::Render(float alpha)
{

}

void SceneSwitcherSystemComponent::RenderUI()
{

}

void SceneSwitcherSystemComponent::Cleanup()
{

}
