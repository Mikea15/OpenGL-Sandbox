#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>

#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_sdl.h"
#include "Dependencies/imgui/imgui_impl_opengl3.h"

#include "WindowParams.h"

namespace Core
{
	class SDLHandler
	{
	public:
		SDLHandler() = default;
		SDLHandler(WindowParams& params);

		~SDLHandler();

		bool Init();

		void HandleEvents(SDL_Event* event);
		void Update(float deltaTime);

		void BeginRender();
		void EndRender();

		void SetWindowParameters(const WindowParams& params, bool initialSetup = false);

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
		WindowParams m_params;

		SDL_Window* m_window;
		SDL_GLContext m_glContext;

		const char* m_glslVersion = "#version 130";

		std::vector<SDL_DisplayMode> m_displayModes;
		std::unordered_map<int, std::string> m_displayModeIndexToName;

		bool m_lockMouseCursor = false;
	};
}

