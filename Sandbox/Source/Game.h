#pragma once

#include <memory>
#include <chrono>

#include "Core/SDLHandler.h"
#include "Core/ImGuiHander.h"
#include "States/State.h"


#include "Managers/AssetManager.h"
#include "Managers/SystemComponentManager.h"
#include "Components/System/SceneCameraComponent.h"
#include "Components/System/ProfilerSystemComponent.h"


class Game
{
public:
	Game();
	~Game();

	int Execute();

	const Core::WindowParameters& GetWindowParameters() const { return m_winParams; }
	Core::SDLHandler& GetSDLHandler() { return m_sdlHandler; }
	SystemComponentManager* GetSystemComponentManager() const { return m_systemComponentManager.get(); }
	AssetManager& GetAssetManager() { return *m_assetManager.get(); }

	void ChangeState(std::shared_ptr<State> newState);
	double GetTimeMS() const { return m_time; }

	void Log(const std::string& className, const std::string& str)
	{
		std::cout << "> " << m_time << " : " << " [" << className << "]" << str << std::endl;
	}

private:
	void InitSystems();
	void CleanupSystems();

private:
	bool m_isRunning;

	std::chrono::high_resolution_clock::time_point m_currentTime;
	double m_deltaTime;
	double m_time;

	std::shared_ptr<State> m_currentState;

	Core::SDLHandler m_sdlHandler;
	Core::ImGuiHandler m_uiHandler;
	Core::WindowParameters m_winParams;
	std::unique_ptr<AssetManager> m_assetManager;

	std::unique_ptr<SystemComponentManager> m_systemComponentManager;
};

