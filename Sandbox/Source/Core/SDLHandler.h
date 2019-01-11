#pragma once

#include <iostream>
#include <memory>

#include <SDL.h>
#include <gl/glew.h>

#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl.h"
#include "Dependencies/imgui/imgui_impl_opengl3.h"

namespace Core
{
	struct WindowParameters
	{
		int Height;
		int Width;
		int Depth;

		int DisplayIndex = 0;

		std::string WindowName = "Sandbox";

		int EnableVSync = 0; // 1 - enabled . 0 - disabled

		int FPSLimit = 90;
	};

	class SDLHandler
	{
	public:
		SDLHandler() = default;
		SDLHandler(WindowParameters params);

		~SDLHandler();

		bool Init();

		void HandleEvents(SDL_Event* event);
		void Update(float deltaTime);

		void BeginRender();
		void EndRender();

		const Uint32 GetTicks() const { return SDL_GetTicks(); }

		SDL_Window* GetSDLWindow() const { return m_window; }
		SDL_GLContext GetGLContext() const { return m_glContext; }


	private:
		WindowParameters m_params;
		SDL_DisplayMode m_displayMode;
		SDL_Window* m_window;
		SDL_GLContext m_glContext;
	};
}

