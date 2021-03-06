
#include "Game.h"

#include <cmath>
#include <algorithm>
#include <sstream>

#include "Managers/Profiler.h"

#include "Components/System/FpsCounterSystemComponent.h"
#include "Components/System/ProfilerSystemComponent.h"
#include "Components/System/SceneSwitcherSystemComponent.h"

const char* Game::s_configFileName = "Levels/config.json";

Game::Game()
	: m_isRunning(true)
	, m_deltaTime(1.0f / 60.0f)
{
	LoadConfig();

	m_sdlHandler = SDLHandler(m_winParams);

	m_systemComponentManager = std::make_unique<SystemComponentManager>();
	m_systemComponentManager->AddComponent<SceneCameraComponent>();
	m_systemComponentManager->AddComponent<FpsCounterSystemComponent>();
	m_systemComponentManager->AddComponent<SceneSwitcherSystemComponent>();
	m_systemComponentManager->AddComponent<ProfilerSystemComponent>();

	// initialize profiler singleton.
	Profiler::Instance().Init(this);
	// Log::Instance().Init(this);
	
	m_assetManager = std::make_unique<AssetManager>();
}

Game::~Game()
{
	SaveConfig();
}

void Game::InitSystems()
{
	srand(static_cast<unsigned int>(time(NULL)));

	m_sdlHandler.Init();

	m_assetManager->Initialize();

	// system components
	m_systemComponentManager->Initialize(this);
}

int Game::Execute()
{
	InitSystems();

	m_currentTime = std::chrono::high_resolution_clock::now();
	float accumulator = 0.0f;

	while (m_isRunning)
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_currentTime);
		float frameTime = static_cast<float>(timeSpan.count());

		m_time += frameTime;

		// cap on how many updates we can do.
		// eg. dt: 1/60 ( 60 ups ).
		// frametime: 0.25
		// max ups = frametime / m_deltaTime
		// max ups = 0.25 / 0.016 = 15.6ups
		// for 30 ups -> max frametime 0.25 ( 7.5 )
		// for 60 ups -> max frametime 0.083 ( 5 updates )
		if (frameTime > 0.25)
			frameTime = 0.25;

		m_currentTime = now;

		accumulator += frameTime;

		m_assetManager->Update();

		m_systemComponentManager->PreUpdate(frameTime);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// Handle Input
			m_sdlHandler.HandleEvents(&event);

			if (event.type == SDL_QUIT) 
			{
				m_isRunning = false;
			}
			if (event.type == SDL_WINDOWEVENT
				&& event.window.event == SDL_WINDOWEVENT_CLOSE
				&& event.window.windowID == SDL_GetWindowID(m_sdlHandler.GetSDLWindow()))
			{
				m_isRunning = false;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			{
				m_isRunning = false;
			}

			// System Components
			m_systemComponentManager->HandleInput(&event);

			m_currentState->HandleInput(&event);
		}

		while (accumulator >= m_deltaTime)
		{
			// system components
			m_systemComponentManager->Update(m_deltaTime);

			// window Update
			m_sdlHandler.Update(m_deltaTime);

			// Update
			m_currentState->Update(m_deltaTime);

			accumulator -= m_deltaTime;
		}

		const float alpha = accumulator / m_deltaTime;
		
		// Render
		m_sdlHandler.BeginRender();
		m_systemComponentManager->Render(alpha);
		m_currentState->Render(alpha);

		// ui
		m_sdlHandler.BeginUIRender();

		m_systemComponentManager->RenderUI();
		m_currentState->RenderUI();

		m_sdlHandler.EndUIRender();
		m_sdlHandler.EndRender();
	}

	CleanupSystems();

	return 0;
}

void Game::CleanupSystems()
{
	// state cleanup
	if (m_currentState)
	{
		m_currentState->Cleanup();
		m_currentState = nullptr;
	}

	// system components
	m_systemComponentManager->Cleanup();
}

void Game::LoadConfig()
{
	std::stringstream stream;
	std::fstream file(s_configFileName, std::fstream::in);
	if (file.is_open()) 
	{
		while (!file.eof())
		{
			std::string buffer;
			std::getline(file, buffer);
			stream << buffer;
		}
	}

	if (!stream.str().empty())
	{
		json jsonParams = json::parse(stream.str());
		if (!jsonParams.is_null()) {
			m_winParams = jsonParams;
		}
	}
}

void Game::SaveConfig()
{
	std::ofstream file;
	file.open(s_configFileName, std::fstream::out);

	file << static_cast<json>(m_winParams).dump(4);
	file.close();
}

void Game::ChangeState(std::shared_ptr<State> newState)
{
	if (m_currentState == nullptr)
	{
		m_currentState = newState;
		m_currentState->Init(this);
	}
	else if (m_currentState != newState)
	{
		m_currentState->Cleanup();
		m_currentState = newState;
		m_currentState->Init(this);
	}
}

