#include "ImGuiHander.h"

#include "Game.h"

namespace Core
{
	ImGuiHandler::ImGuiHandler()
	{

	}

	void ImGuiHandler::Initialize(Game* game)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		m_io = ImGui::GetIO();

		m_sdlHandler = game->GetSDLHandler();

		ImGui_ImplSDL2_InitForOpenGL(
			m_sdlHandler->GetSDLWindow(),
			m_sdlHandler->GetGLContext()
		);
		ImGui_ImplOpenGL3_Init("#version 130");
		ImGui::StyleColorsDark();
	}

	void ImGuiHandler::HandleInput(SDL_Event * event)
	{
		ImGui_ImplSDL2_ProcessEvent(event);
	}

	void ImGuiHandler::StartRender()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_sdlHandler->GetSDLWindow());
		ImGui::NewFrame();
	}

	void ImGuiHandler::EndRender()
	{
		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiHandler::Cleanup()
	{
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

}
