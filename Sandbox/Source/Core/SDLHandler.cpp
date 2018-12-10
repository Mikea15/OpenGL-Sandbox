#include "SDLHandler.h"

namespace Core
{
	SDLHandler::SDLHandler(WindowParameters params)
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

		const char* glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

		// Create window with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, m_params.Depth);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		SDL_GetCurrentDisplayMode(m_params.DisplayIndex, &m_displayMode);

		m_window = SDL_CreateWindow(m_params.WindowName.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, m_params.Width, m_params.Height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		SDL_SetWindowResizable(m_window, SDL_TRUE);

		m_glContext = SDL_GL_CreateContext(m_window);
		SDL_GL_SetSwapInterval(m_params.EnableVSync);

		glewInit();

		// have this by default
		glEnable(GL_DEPTH_TEST);

		return true;
	}

	void SDLHandler::HandleEvents(SDL_Event * event)
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

}