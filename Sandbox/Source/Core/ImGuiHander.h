#pragma once

#include "SDLHandler.h"

class Game;

namespace Core
{
	class ImGuiHandler
	{
	public:
		ImGuiHandler();
		~ImGuiHandler() = default;

		void Initialize(Game* game);
		void HandleInput(SDL_Event* event);

		void StartRender();
		void EndRender();

		void Cleanup();

		const ImGuiIO& GetIO() const { return m_io; }

	private:
		ImGuiIO m_io;
		const Core::SDLHandler* m_sdlHandler;
	};
}
