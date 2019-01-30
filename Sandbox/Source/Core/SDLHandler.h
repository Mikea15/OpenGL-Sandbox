#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>

#include <SDL.h>
#include <gl/glew.h>

#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl.h"
#include "Dependencies/imgui/imgui_impl_opengl3.h"

namespace Core
{
	struct WindowParameters
	{
		// Window
		int Height = 600;
		int Width = 800;
		int Depth = 24;
		int ResolutionIndex = 0;
		bool Fullscreen = false;
		int DisplayIndex = 0;

		std::string WindowName = "Sandbox";

		// 0 immediate, 1 wait for sync
		bool VSync = 0;

		int FPSLimit = 90;

		// OpenGL
		int GL_Flags = 0;
		SDL_GLprofile GL_ProfileMask = SDL_GLprofile::SDL_GL_CONTEXT_PROFILE_CORE;
		int GL_MajorVersion = 4;
		int GL_MinorVersion = 5;
		int GL_DoubleBuffer = 1;
		int GL_Accelerated = 1;
		int GL_DepthSize = 24;
		int GL_StencilSize = 8;
		int GL_MultiSampleBuffers = 1;
		int GL_MultiSamplesSamples = 4;
	};

	class SDLHandler
	{
	public:
		SDLHandler() = default;
		SDLHandler(WindowParameters& params);

		~SDLHandler();

		bool Init();

		void HandleEvents(SDL_Event* event);
		void Update(float deltaTime);

		void BeginRender();
		void EndRender();

		void SetWindowParameters(const WindowParameters& params, bool initialSetup = false);

		const Uint32 GetTicks() const { return SDL_GetTicks(); }

		SDL_Window* GetSDLWindow() const { return m_window; }
		SDL_GLContext GetGLContext() const { return m_glContext; }

		const char* GetGLSLVersion() const { return m_glslVersion; }

		const std::vector<SDL_DisplayMode>& GetDisplayModes() const { return m_displayModes; }
		const std::string& GetDisplayModeName(int index) const { 
			auto it = m_displayModeIndexToName.find(index);
			if (it != m_displayModeIndexToName.end() ) { return it->second; }
			return "";
		}
		const SDL_DisplayMode* GetCurrentDisplayMode();

	private:
		void FindDisplayModes();

	private:
		WindowParameters m_params;

		SDL_Window* m_window;
		SDL_GLContext m_glContext;

		const char* m_glslVersion = "#version 130";

		std::vector<SDL_DisplayMode> m_displayModes;
		std::unordered_map<int, std::string> m_displayModeIndexToName;

		bool m_lockMouseCursor = false;
	};
}

