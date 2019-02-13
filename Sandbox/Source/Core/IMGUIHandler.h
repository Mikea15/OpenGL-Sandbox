#pragma once

#include <SDL.h>

#include "Dependencies/imgui/imgui.h"


class IMGUIHandler
{
public:
	IMGUIHandler();
	~IMGUIHandler() = default;

	void Initialize(SDL_Window* window, SDL_GLContext* glContext, const char* glslVersion);
	void HandleInput(SDL_Event* event);

	void StartRender(SDL_Window* window);
	void EndRender();

	void Cleanup();

	const ImGuiIO& GetIO() const { return m_io; }

private:
	ImGuiIO m_io;
};

