#include "ShadowMappingState.h"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Systems/Rendering/Primitives.h"

#include "Game.h"


ShadowMappingState::ShadowMappingState()
{
	m_shaders = ShaderManager();

	shader = m_shaders.LoadShader("pointshadow", "point_shadows.vs", "point_shadows.fs");
	simpleDepthShader = m_shaders.LoadShader("pointshadow_depth", "point_shadows_depth.vs", "point_shadows_depth.fs", "point_shadows_depth.gs");
	multipleLightsShader = m_shaders.GetShader("lighting");
	skyboxShader = m_shaders.GetShader("skybox");
	reflectionShader = m_shaders.GetShader("cubemaps");
	refractionShader = m_shaders.GetShader("refraction");
	hdrShader = m_shaders.GetShader("hdr");
}

void ShadowMappingState::Init(Game* game)
{
	m_windowParams = game->GetWindowParameters();
	m_sdlHandler = game->GetSDLHandler();

	m_sceneCameraComp = &game->GetSystemComponentManager()->GetComponent<SceneCameraComponent>();
	AssetManager& assetManager = game->GetAssetManager();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	skyboxShader.Use();
	skyboxShader.SetInt("skybox", 0);

	reflectionShader.Use();
	reflectionShader.SetInt("skybox", 0);

	refractionShader.Use();
	refractionShader.SetInt("skybox", 0);

	hdrShader.Use();
	hdrShader.SetInt("hdrBuffer", 0);

	m_skybox = Skybox();

	cubeObject = Entity();
	
	//cubeObject.SetModel(assetManager.LoadModel("Data/Objects/default/cube1m.fbx"));
	cubeObject.GetTransform().Translate(glm::vec3(-5.0f, 1.0f, 0.0f));

	sphereObject = Entity();
	//sphereObject.SetModel(assetManager.LoadModel("Data/Objects/default/sphere.fbx"));
	sphereObject.GetTransform().Translate(glm::vec3(-2.5f, 1.0f, 0.0f));

	metalSphere = Entity();
	//metalSphere.SetModel(assetManager.LoadModel("Data/Objects/default/sphere.fbx"));
	metalSphere.GetTransform().Translate(glm::vec3(0.0f, 1.0f, 0.0f));
	
	refractSphere = Entity();
	//refractSphere.SetModel(assetManager.LoadModel("Data/Objects/default/sphere.fbx"));
	refractSphere.GetTransform().Translate(glm::vec3(2.5f, 1.0f, 0.0f));

	plane = Entity();
	//plane.SetModel(assetManager.LoadModel("Data/Objects/default/plane10m.fbx"));
	
	woodTexture = assetManager.LoadTexture("Data/Images/wood.png", false).id;
	uvTexture = assetManager.LoadTexture("Data/Objects/default/uv_grid.jpg", false).id;

	m_plane = Plane();
	m_plane.GenerateMesh();
	planeTransform = Transform();
	planeTransform.Translate(glm::vec3(-10.0f, 0.0f, -10.0f));
	
	TextureBuffer::CreateDepthCubeMap(SHADOW_WIDTH, SHADOW_HEIGHT, depthMapFBO, depthCubemap);
	TextureBuffer::CreateFrameBuffer(m_windowParams.Width, m_windowParams.Height, hrdFrameBufferObject, colorBuffer, rboDepth);
	TextureBuffer::CreateDepthMapFBO(m_windowParams.Width, m_windowParams.Height, depthFBO, depthMap);

	// shader configuration
	// --------------------
	shader.Use();
	shader.SetInt("diffuseTexture", 0);
	shader.SetInt("depthMap", 1);

	multipleLightsShader.Use();
	multipleLightsShader.SetInt("diffuseTexture", 0);

	// lighting info
	// -------------
	lightPos = glm::vec3(0.0f, 5.0f, 0.0f);

	lightPositions.push_back(glm::vec3(0.0f, 10.0f, 0.0f)); // back light
	lightPositions.push_back(glm::vec3(-2.5f, 2.5f, 0.0f));
	lightPositions.push_back(glm::vec3(-3.0f, 1.8f, 0.0f));
	lightPositions.push_back(glm::vec3(0.8f, 1.7f, -1.0f));

	lightColors.push_back(glm::vec3(20.0f, 20.0f, 20.0f));
	lightColors.push_back(glm::vec3(0.5f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 0.8f));
	lightColors.push_back(glm::vec3(0.0f, 0.9f, 0.0f));

	ImGui::StyleColorsDark();
}

void ShadowMappingState::HandleInput(SDL_Event* event)
{
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_f) shadows = !shadows;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_h) isHdrEnabled = !isHdrEnabled;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_u) exposure += 0.05f;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_j) {
		if (exposure > 0.0f)
			exposure -= 0.05f;
		if (exposure <= 0.0f)
			exposure = 0.0f;
	}
		
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_t) {
		cubeObject.GetTransform().Scale(glm::vec3(1.0f), 0.1f);
	}
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_y) {
		cubeObject.GetTransform().Scale(glm::vec3(1.0f), -0.1f);
	}
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_g) {
		sphereObject.GetTransform().RotateLocal(glm::vec3(0, 1, 0), -300);
	}
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_h) {
		sphereObject.GetTransform().RotateLocal(glm::vec3(0, 1, 0), 300);
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_v) {
		m_refractionIndex += 0.1f;
	}
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_b) {
		m_refractionIndex -= 0.1f;
		m_refractionIndex = std::max(1.0f, m_refractionIndex);
	}

	if (event->type == SDL_MOUSEWHEEL)
	{
		//camera.ProcessMouseScroll(static_cast<float>(event->wheel.y));
	}
}

void ShadowMappingState::Update(float deltaTime)
{
	this->deltaTime = deltaTime;
	totalTime += deltaTime;

	const float amplitude = 1.0f;
	const float frequency = 5.0f;

	glm::vec3 movement = glm::vec3(
		amplitude * cosf(totalTime * frequency),
		amplitude * sinf(totalTime * frequency),
		0.0f);
}

void ShadowMappingState::Render(float alpha)
{
	glm::mat4 projection = m_sceneCameraComp->GetCamera().ProjectionMatrix();
	glm::mat4 view = m_sceneCameraComp->GetCamera().View();

	// render
	// ------
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//// 0. create depth cubemap transformation matrices
	//// -----------------------------------------------
	//float near_plane = 1.0f;
	//float far_plane = 25.0f;
	//glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
	//std::vector<glm::mat4> shadowTransforms;
	//shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	//shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	//shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	//shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	//shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	//shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	//
	//// 1. render scene to depth cubemap
	//// --------------------------------
	//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glClear(GL_DEPTH_BUFFER_BIT);
	//simpleDepthShader.Use();
	//for (unsigned int i = 0; i < 6; ++i)
	//	simpleDepthShader.SetMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	//simpleDepthShader.SetFloat("far_plane", far_plane);
	//simpleDepthShader.SetVec3("lightPos", lightPos);
	//renderScene(simpleDepthShader);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 1a. render scene into floating point framebuffer
	// -----------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, hrdFrameBufferObject);

	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	multipleLightsShader.Use();
	multipleLightsShader.SetMat4("projection", projection);
	multipleLightsShader.SetMat4("view", view);
	// set lighting uniforms
	multipleLightsShader.SetVec3("lightPos", lightPos);

	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
		multipleLightsShader.SetVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
		multipleLightsShader.SetVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
	}

	multipleLightsShader.SetVec3("viewPos", m_sceneCameraComp->GetCamera().GetPosition());
	// shader.SetInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
	// shader.SetFloat("far_plane", camera.GetFarPlane());
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	//glActiveTexture(GL_TEXTURE0);
	
	multipleLightsShader.SetMat4("model", cubeObject.GetTransform().GetModelMat());
	cubeObject.GetModel().Draw(multipleLightsShader);

	multipleLightsShader.SetMat4("model", sphereObject.GetTransform().GetModelMat());
	sphereObject.GetModel().Draw(multipleLightsShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uvTexture);
	multipleLightsShader.SetMat4("model", planeTransform.GetModelMat());
	m_plane.GetMesh().Draw(multipleLightsShader);
	glActiveTexture(GL_TEXTURE0);

	reflectionShader.Use();
	reflectionShader.SetMat4("view", view);
	reflectionShader.SetMat4("projection", projection);
	reflectionShader.SetMat4("model", metalSphere.GetTransform().GetModelMat());
	reflectionShader.SetVec3("cameraPos", m_sceneCameraComp->GetCamera().GetPosition());
	metalSphere.GetModel().Draw(reflectionShader);
	
	refractionShader.Use();
	refractionShader.SetMat4("view", view);
	refractionShader.SetMat4("projection", projection);
	refractionShader.SetMat4("model", refractSphere.GetTransform().GetModelMat());
	refractionShader.SetFloat("refraction", m_refractionIndex);
	refractionShader.SetVec3("cameraPos", m_sceneCameraComp->GetCamera().GetPosition());
	refractSphere.GetModel().Draw(refractionShader);
	
	// - skybox
	glDepthFunc(GL_LEQUAL);
	skyboxShader.Use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1000.0f));
	skyboxShader.SetMat4("model", model);
	skyboxShader.SetMat4("view", view);
	skyboxShader.SetMat4("projection", projection);
	glBindVertexArray(m_skybox.GetVAO());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID());
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 2. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
	// --------------------------------------------------------------------------------------------------------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	hdrShader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	hdrShader.SetInt("hdr", isHdrEnabled);
	hdrShader.SetFloat("exposure", exposure);
	Primitives::RenderQuad();
	// renderQuad();

	// std::cout << "hdr: " << (isHdrEnabled ? "on" : "off") << "| exposure: " << exposure << std::endl;
	// 2. render scene as normal 
	// -------------------------
	// glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 
	// if (wireframeMode) 
	// {
	// 	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// }
	// else
	// {
	// 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// }
	// 
	// shader.Use();
	// shader.SetMat4("projection", projection);
	// shader.SetMat4("view", view);
	// // set lighting uniforms
	// shader.SetVec3("lightPos", lightPos);
	// shader.SetVec3("viewPos", camera.GetPosition());
	// shader.SetInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
	// shader.SetFloat("far_plane", camera.GetFarPlane());
	// 
	// // glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, woodTexture);
	// glActiveTexture(GL_TEXTURE1);
	// glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	// glActiveTexture(GL_TEXTURE0);
	// // renderScene(shader);
	// 
	// shader.SetMat4("model", cubeObject.GetTransform().GetModelMat());
	// cubeObject.GetModel().Draw(shader);
	// 
	// shader.SetMat4("model", sphereObject.GetTransform().GetModelMat());
	// sphereObject.GetModel().Draw(shader);
	// 
	// reflectionShader.Use();
	// reflectionShader.SetMat4("view", view);
	// reflectionShader.SetMat4("projection", projection);
	// reflectionShader.SetMat4("model", metalSphere.GetTransform().GetModelMat());
	// reflectionShader.SetVec3("cameraPos", camera.GetPosition());
	// metalSphere.GetModel().Draw(reflectionShader);
	// 
	// refractionShader.Use();
	// refractionShader.SetMat4("view", view);
	// refractionShader.SetMat4("projection", projection);
	// refractionShader.SetMat4("model", refractSphere.GetTransform().GetModelMat());
	// refractionShader.SetFloat("refraction", m_refractionIndex);
	// refractionShader.SetVec3("cameraPos", camera.GetPosition());
	// refractSphere.GetModel().Draw(refractionShader);
	// 
	// shader.Use();
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, uvTexture);
	// shader.SetMat4("model", planeTransform.GetModelMat());
	// m_plane.GetMesh().Draw(shader);
	// glActiveTexture(GL_TEXTURE0);

	// - skybox
	// glDepthFunc(GL_LEQUAL);
	// skyboxShader.Use();
	// glm::mat4 model = glm::mat4(1.0f);
	// model = glm::scale(model, glm::vec3(100.0f));
	// skyboxShader.SetMat4("model", model);
	// skyboxShader.SetMat4("view", view);
	// skyboxShader.SetMat4("projection", projection);
	// glBindVertexArray(m_skybox.GetVAO());
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID());
	// glDrawArrays(GL_TRIANGLES, 0, 36);
	// glBindVertexArray(0);
	// glDepthFunc(GL_LESS);

}

void ShadowMappingState::RenderUI()
{
	ImGui::Begin("Rendering Settings"); ImGui::BeginGroup();

	ImGui::Checkbox("High Dynamic Range", &isHdrEnabled);
	ImGui::SliderFloat("Exposure", &exposure, 0.0f, 5.0f);

	ImGui::EndGroup(); ImGui::End();

	ImGui::Begin("Terrain Settings");

	ImGui::BeginGroup();
	ImGui::Text("Terrain Size");
	glm::vec2 planeSize = m_plane.GetPlaneSize();
	ImGui::SliderFloat("Width", &planeSize.x, 1.0f, 256.0f);
	ImGui::SliderFloat("Length", &planeSize.y, 1.0f, 256.0f);

	float heightSize = m_plane.GetHeightSize();
	ImGui::SliderFloat("Height", &heightSize, 1.0f, 64.0f);
	if (heightSize != m_plane.GetHeightSize())
	{
		m_plane.SetHeightSize(heightSize);
		m_plane.UpdateHeightMap();
	}

	if (planeSize != m_plane.GetPlaneSize())
	{
		m_plane.SetPlaneSize(planeSize);
	}

	if (ImGui::Button("Regenerate Terrain")) 
	{
		m_plane.GenerateMesh();
	}

	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Text("Heightmap Settings");

	HeightmapParams currentParams = m_plane.GetHeightMapParams();
	ImGui::SliderInt("Octaves", &currentParams.octaves, 1, 10);
	ImGui::SliderFloat("Lacunarity", &currentParams.lacunarity, 0.0f, 1.0f);
	ImGui::SliderFloat("Persistence", &currentParams.persistence, 0, 3);
	if (currentParams != m_plane.GetHeightMapParams())
	{
		m_plane.SetHeightMapParams(currentParams);
	}

	ImGui::EndGroup();
	ImGui::End();
}

void ShadowMappingState::Cleanup()
{

}
