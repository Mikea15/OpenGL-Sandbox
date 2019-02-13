#include "OcclusionState.h"

#include "Game.h"
#include "Core/Utils.h"
#include "Systems/Rendering/Primitives.h"

#include <random>

static float ccCamNearPlane = 1.0f;
static float ccCamFarPlane = 10.0f;
static float ccCamFov = 60.0f;
static float ccRotationSpeed = 10.0f;

void OcclusionState::Init(Game* game)
{
	DefaultState::Init(game);

	backgroundShader = shaderManager.LoadShader("backgroundShader", "pbr/background.vs", "pbr/background.fs");
	wireframeShader = shaderManager.LoadShader("wireframeSimple", "unlit/simple.vs", "unlit/color.fs");
	
	wireframeShader.Use();
	wireframeShader.SetVec3("Color", glm::vec3(0, 0, 1.0));

	backgroundShader.Use();
	backgroundShader.SetInt("environmentMap", 0);

	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(-10, 10); // define the range

	const unsigned int nCubes = 10;
	for (unsigned int i = 0; i < nCubes; i++)
	{
		glm::vec3 v3 = glm::vec3(distr(eng), 0.0f, distr(eng));
		positions.push_back(v3);
		visible.push_back(ContainmentType::Disjoint);

		m_qTree.Insert(v3);
		m_oTree.Insert(v3);
	}

	cc.SetNearFarPlane(ccCamNearPlane, ccCamFarPlane);
	cc.SetFov(ccCamFov);
}

void OcclusionState::HandleInput(SDL_Event * event)
{
	DefaultState::HandleInput(event);
}

void OcclusionState::Update(float deltaTime)
{
	DefaultState::Update(deltaTime);

	cc.UpdateLookAt(glm::vec2(1.0f, 0.0f) * ccRotationSpeed * deltaTime);
	cc.Update(deltaTime);
}

void OcclusionState::Render(float alpha)
{
	// make sure we clear the framebuffer's content
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// configure transformation matrices
	glm::mat4 view = m_sceneCamera->GetCamera().GetView();
	glm::mat4 projection = m_sceneCamera->GetCamera().GetProjection();
	glm::vec3 cameraPosition = m_sceneCamera->GetCamera().GetPosition();

	wireframeShader.Use();
	wireframeShader.SetMat4("view", view);
	wireframeShader.SetMat4("projection", projection);
	wireframeShader.SetVec3("camPos", cameraPosition);

	// draw 3d grid of cubes.
	const unsigned int size = positions.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		scratchTransform.SetPosition(positions[i]);

		wireframeShader.SetMat4("model", scratchTransform.GetModelMat());
		visible[i] = cc.GetBoundingFrustum().Contains(BoundingBox(positions[i], 1.0f));

		if (visible[i] == ContainmentType::Disjoint)
		{
			wireframeShader.SetVec3("Color", glm::vec3(0.1f, 0.2f, 0.5f));

			Primitives::RenderSphere(true);
		}
		else if ( visible[i] == ContainmentType::Contains )
		{
			wireframeShader.SetVec3("Color", glm::vec3(0.1f, 1.0f, 0.5f));
			Primitives::RenderCube(true);
		}
		else
		{
			wireframeShader.SetVec3("Color", glm::vec3(1.0f, 1.0f, 1.0f));
			Primitives::RenderCube(false);
		}
	}

	glm::vec3 ccP = cc.GetPosition();
	auto points = cc.GetBoundingFrustum().GetCorners();
	// near
	Primitives::RenderLine(points[0], points[3]);
	Primitives::RenderLine(points[1], points[0]);
	Primitives::RenderLine(points[2], points[1]);
	Primitives::RenderLine(points[3], points[2]);

	// far
	Primitives::RenderLine(points[4], points[7]);
	Primitives::RenderLine(points[5], points[4]);
	Primitives::RenderLine(points[6], points[5]);
	Primitives::RenderLine(points[7], points[6]);

	// near to far
	Primitives::RenderLine(points[0], points[4]);
	Primitives::RenderLine(points[1], points[5]);
	Primitives::RenderLine(points[2], points[6]);
	Primitives::RenderLine(points[3], points[7]);
}

void OcclusionState::RenderUI()
{
	DefaultState::RenderUI();

	ImGui::Begin("CC Settings");

	ImGui::SliderFloat("Rotation Speed", &ccRotationSpeed, 0.0f, 100.0f);
	ImGui::SliderFloat("Near plane", &ccCamNearPlane, 0.1f, ccCamFarPlane);
	ImGui::SliderFloat("Far plane", &ccCamFarPlane, ccCamNearPlane, 1000.0f);
	ImGui::SliderFloat("FOV", &ccCamFov, 0.1f, 179.0f);

	cc.SetFov(ccCamFov);
	cc.SetNearFarPlane(ccCamNearPlane, ccCamFarPlane);

	ImGui::End();
}

void OcclusionState::Cleanup()
{
}
