#include "ProfilerSystemComponent.h"

CLASS_DEFINITION(SystemComponent, ProfilerSystemComponent)

ProfilerSystemComponent::ProfilerSystemComponent()
{

}

ProfilerSystemComponent::~ProfilerSystemComponent()
{
}

void ProfilerSystemComponent::Initialize(Game* game)
{
	m_profileData = Profiler::Instance().GetProfileData();
}

void ProfilerSystemComponent::HandleInput(SDL_Event* event)
{
	
}

void ProfilerSystemComponent::PreUpdate(float frameTime)
{
	
}

void ProfilerSystemComponent::Update(float deltaTime)
{
	
}

void ProfilerSystemComponent::Render(float alpha)
{
	
}

void ProfilerSystemComponent::RenderUI()
{
	ImGui::Begin("Time Profiler");
	
	
	ImGui::Bullet(); ImGui::TextWrapped("Horizontal scrolling for a window has to be enabled explicitly via the ImGuiWindowFlags_HorizontalScrollbar flag.");
	
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
	ImGui::BeginChild("scrolling", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30), true, ImGuiWindowFlags_HorizontalScrollbar);
		
		// Display random stuff (for the sake of this trivial demo we are using basic Button+SameLine. If you want to create your own time line for a real application you may be better off
		// manipulating the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets yourself. You may also want to use the lower-level ImDrawList API)
		const auto& dataRef = *m_profileData;
		const int size = dataRef.size();
		const int margin = 5;
		const int hSize = 20;
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::BeginGroup();
		for (size_t i = 0; i < 10; i++)
		{
			int rSize = 5 * i;
			ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + rSize, pos.y + hSize), IM_COL32(90, 90, 120, 255));
			ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x, pos.y + 2), IM_COL32(255, 255, 255, 255), "Hi");
			pos.x += rSize + margin;
		}

		// for (unsigned int n = 0; n < size; ++n)
		// {
		// 	const auto& d = dataRef[n];
		// 	ImVec2 pos(n * d.startMS, 5);
		// 	ImVec2 rectSize(d.durationMS + 20, 20);
		// 	ImGui::InvisibleButton("##dummy", rectSize);
		// 	ImGui::GetWindowDrawList()->AddRectFilled(pos, rectSize, IM_COL32(90, 90, 120, 255));
		// 	ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, IM_COL32(255, 255, 255, 255), d.name.c_str());
		// }
		ImGui::EndGroup();

		float scroll_x = ImGui::GetScrollX(), scroll_max_x = ImGui::GetScrollMaxX();

	ImGui::EndChild();
		ImGui::PopStyleVar(2);

		ImGui::SetScrollX(scroll_max_x);

		float scroll_x_delta = 0.0f;
		ImGui::SmallButton("<<"); if (ImGui::IsItemActive()) scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f; ImGui::SameLine();
		ImGui::Text("Scroll from code"); ImGui::SameLine();
		ImGui::SmallButton(">>"); if (ImGui::IsItemActive()) scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f; ImGui::SameLine();
		ImGui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
		if (scroll_x_delta != 0.0f)
		{
			ImGui::BeginChild("scrolling"); // Demonstrate a trick: you can use Begin to set yourself in the context of another window (here we are already out of your child window)
			ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
			ImGui::End();
		}

	
	
	ImGui::End();
}

void ProfilerSystemComponent::Cleanup()
{

}
