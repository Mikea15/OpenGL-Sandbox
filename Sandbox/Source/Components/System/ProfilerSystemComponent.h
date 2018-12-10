#pragma once


#include "Game.h"
#include "Managers/Profiler.h"
#include "Managers/SystemComponentManager.h"

struct ProfileData;

class ProfilerSystemComponent
	: public SystemComponent
{
	CLASS_DECLARATION(ProfilerSystemComponent)

public:
	ProfilerSystemComponent();
	~ProfilerSystemComponent();

	void Initialize(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void PreUpdate(float frameTime) override;
	void Update(float deltaTime) override;
	void Render(float alpha) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	std::vector<ProfileData>* m_profileData;
};