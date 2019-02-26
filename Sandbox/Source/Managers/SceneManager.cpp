#include "SceneManager.h"

#include "Dependencies/imgui/imgui.h"

#include "Systems/Entity.h"

void InputVec3(const char* label, glm::vec3& outVec, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
{
	float vec[3] = { outVec.x, outVec.y, outVec.z };
	ImGui::SliderFloat3(label, vec, -10000.0f, 10000.0f);
	outVec = glm::vec3(vec[0], vec[1], vec[2]);
}

void InputQuat(const char* label, glm::quat& outVec, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
{
	float vec[4] = { outVec.w, outVec.x, outVec.y, outVec.z };
	ImGui::SliderFloat4(label, vec, -10000.0f, 10000.0f);
	outVec = glm::quat(vec[0], vec[1], vec[2], vec[3]);
}

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::AddEntity(std::shared_ptr<Entity> entity)
{
	m_sceneObjects.push_back(entity);
}


void SceneManager::Update(float deltaTime)
{
	for (auto entity : m_sceneObjects)
	{
		// entity->Update(deltaTime);
	}
}

void SceneManager::Draw()
{
	for (auto entity : m_sceneObjects)
	{
		entity->Draw();
	}
}

void SceneManager::RenderUI()
{
	ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
	ImGui::Begin("Hierachy");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();

	struct funcs
	{
		static void ShowTransformInfo(Transform& transform, int uid)
		{
			ImGui::PushID(uid);
			ImGui::AlignTextToFramePadding();
			bool node_open = ImGui::TreeNode("Transform", "%u", uid);

			if (node_open)
			{
				ImGui::NextColumn();
				ImGui::Text("Transform");
				ImGui::Separator();

				glm::vec3 pos = transform.GetPosition();
				glm::vec3 scale = transform.GetScale();
				glm::quat rot = transform.GetOrientation();

				InputVec3("Position", pos);
				InputQuat("Rotation", rot);
				InputVec3("Scale", scale);

				transform.SetPosition(pos);
				transform.SetScale(scale);
				transform.SetOrientation(rot);

				ImGui::TreePop();
			}

			ImGui::PopID();
		};

		static void ShowDummyObject(const char* prefix, int uid)
		{
			ImGui::PushID(uid);                // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
			ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
			bool node_open = ImGui::TreeNode("Object", "%s_%u", prefix, uid);
			ImGui::NextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Demo Content");
			ImGui::NextColumn();
			if (node_open)
			{
				static float dummy_members[8] = { 0.0f,0.0f,1.0f,3.1416f,100.0f,999.0f };
				for (int i = 0; i < 8; i++)
				{
					ImGui::PushID(i); // Use field index as identifier.
					if (i < 2)
					{
						ShowDummyObject("Child", 424242);
					}
					else
					{
						// Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
						ImGui::AlignTextToFramePadding();
						ImGui::TreeNodeEx("Field", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "Field_%d", i);
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (i >= 5)
							ImGui::InputFloat("##value", &dummy_members[i], 1.0f);
						else
							ImGui::DragFloat("##value", &dummy_members[i], 0.01f);
						ImGui::PopItemWidth();
						ImGui::NextColumn();
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	};

	int i = 0;
	for (auto entity : m_sceneObjects)
	{
		funcs::ShowTransformInfo(entity->GetTransform(), i);
		++i;
	}

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();
	ImGui::End();
}
