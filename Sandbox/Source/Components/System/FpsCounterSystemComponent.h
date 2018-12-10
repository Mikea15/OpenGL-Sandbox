#pragma once

#include "Managers/SystemComponentManager.h"

class FpsCounterSystemComponent
	: public SystemComponent
{
	CLASS_DECLARATION(FpsCounterSystemComponent)

public:
	FpsCounterSystemComponent();
	~FpsCounterSystemComponent();

	void Initialize(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void PreUpdate(float frameTime) override;
	void Update(float deltaTime) override;
	void Render(float alpha) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	float m_totalTime = 0;
	float m_framesCicles = 0;
	float m_updateCicles = 0;
	float m_oneSecond{};
	float m_updatesPerSecond{};
	float m_framesPerSecond{};
	float m_minFps = 9999.0f;
	float m_maxFps = 0.0f;

	float m_minFrameTime = 9999.0f;
	float m_maxFrameTime = 0.0f;

	float m_lastFrameTimeMS{};
	float m_lastDeltaTime{};

	const int nMsBuffer = 60;
	int m_currentMsBufferIndex = 0;
	float m_msBuffer[60] = { 0 };

	const int nFpsBuffer = 60;
	int m_currentFpsBufferIndex = 0;
	float m_fpsBuffer[60] = { 0 };

	float m_alpha = 0.0f;

	bool show_demo_window = false;
	bool show_another_window = false;
	bool open = true;
};