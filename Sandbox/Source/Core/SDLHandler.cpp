#include "SDLHandler.h"

#include <string>

namespace Core
{
	SDLHandler::SDLHandler(WindowParameters& params)
		: m_params(params)
	{
	}

	SDLHandler::~SDLHandler()
	{
		SDL_GL_DeleteContext(m_glContext);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	bool SDLHandler::Init()
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
		{
			std::cerr << "SDL Init Error" << std::endl;
			return false;
		}

		FindDisplayModes();

		// set initial params. don't want to pickup best mode for dev.
		const int width = m_params.Width;
		const int height = m_params.Height;

		m_window = SDL_CreateWindow(
			m_params.WindowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		const bool initialSetup = true;
		SetWindowParameters(m_params, initialSetup);

		m_glContext = SDL_GL_CreateContext(m_window);
		
		glewInit();

		// have this by default
		glEnable(GL_DEPTH_TEST);

		return true;
	}

	void SDLHandler::HandleEvents(SDL_Event* event)
	{
		if (event->window.event == SDL_WINDOWEVENT_RESIZED)
		{
			m_params.Width = static_cast<int>(event->window.data1);
			m_params.Height = static_cast<int>(event->window.data2);
			SDL_SetWindowSize(m_window, m_params.Width, m_params.Height);
		}
	}

	void SDLHandler::Update(float deltaTime)
	{

	}

	void SDLHandler::BeginRender()
	{
		SDL_GL_MakeCurrent(m_window, m_glContext);
	}

	void SDLHandler::EndRender()
	{
		SDL_GL_SwapWindow(m_window);
	}

	void SDLHandler::SetWindowParameters(const WindowParameters& params, bool initialSetup)
	{
		m_params = params;

		if (!initialSetup) 
		{
			const SDL_DisplayMode* dm = GetCurrentDisplayMode();
			if (dm != nullptr)
			{
				SDL_SetWindowDisplayMode(m_window, dm);
				SDL_SetWindowSize(m_window, dm->w, dm->h);
				m_params.Width = dm->w;
				m_params.Height = dm->h;
			}
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, m_params.GL_Flags);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, m_params.GL_ProfileMask);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, m_params.GL_MajorVersion);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, m_params.GL_MinorVersion);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, m_params.GL_DoubleBuffer);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, m_params.GL_Accelerated);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, m_params.GL_DepthSize);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, m_params.GL_StencilSize);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, m_params.GL_MultiSampleBuffers);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, m_params.GL_MultiSamplesSamples);

		SDL_SetWindowFullscreen(m_window, m_params.Fullscreen);
		SDL_SetWindowResizable(m_window, SDL_TRUE);

		SDL_GL_SetSwapInterval(m_params.VSync);
	}

	const SDL_DisplayMode* SDLHandler::GetCurrentDisplayMode()
	{
		SDL_DisplayMode* displayMode = nullptr;
		const unsigned int displayModesSize = m_displayModes.size();
		if (m_params.ResolutionIndex >= 0 && m_params.ResolutionIndex < displayModesSize)
		{
			displayMode = &m_displayModes[m_params.ResolutionIndex];
		}
		return displayMode;
	}

	void SDLHandler::FindDisplayModes()
	{
		SDL_DisplayMode currentDisplayMode;
		SDL_GetCurrentDisplayMode(m_params.DisplayIndex, &currentDisplayMode);

		const unsigned int nDisplayModes = SDL_GetNumDisplayModes(m_params.DisplayIndex);
		for (unsigned int i = 0; i < nDisplayModes; ++i)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(m_params.DisplayIndex, i, &mode);

			if (mode.w == currentDisplayMode.w &&
				mode.h == currentDisplayMode.h &&
				mode.refresh_rate == currentDisplayMode.refresh_rate)
			{
				m_params.ResolutionIndex = i;
			}

			std::string st = std::to_string(mode.w)
				+ std::string(" x ")
				+ std::to_string(mode.h)
				+ std::string(" @ ")
				+ std::to_string(mode.refresh_rate)
				+ std::string("hz ")
				+ std::to_string(mode.format);

			m_displayModeIndexToName.emplace(i, st);
			m_displayModes.push_back(mode);
		}
	}

}