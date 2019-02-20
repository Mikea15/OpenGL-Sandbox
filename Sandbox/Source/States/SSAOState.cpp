#include "SSAOState.h"

#include "Game.h"

SSAOState::SSAOState()
{

}

SSAOState::~SSAOState()
{

}

void SSAOState::Init(Game* game)
{
	DefaultState::Init(game);

	shaderGeometryPass = shaderManager.LoadShader("ssao_geometry", "ssao_geometry.vs", "ssao_geometry.fs");
	shaderLightingPass = shaderManager.LoadShader("ssao_lighting", "screen/ssao.vs", "ssao_lighting.fs");
	shaderSSAO = shaderManager.LoadShader("ssao", "screen/ssao.vs", "screen/ssao.fs");
	shaderSSAOBlur = shaderManager.LoadShader("ssaoBlur", "screen/ssao.vs", "screen/ssao_blur.fs");

	ent = Entity();
	Model model;
	m_assetManager->LoadModel("Data/Objects/sponza/sponza.obj", model);
	ent.SetModel(model);
	ent.GetTransform().SetScale(glm::vec3(0.01f));

	// objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	// objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	// objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	// objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	// objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	// objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	// objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	// objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	// objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));

	// configure g-buffer framebuffer
	// ------------------------------
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &renderBufferObjectDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObjectDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_windowParams.Width, m_windowParams.Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferObjectDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ssaoFx.GenBuffers(m_windowParams.Width, m_windowParams.Height);
	ssaoFx.SetupShaders(shaderSSAO, shaderSSAOBlur);

	// setup shaders
	shaderLightingPass.Use();
	shaderLightingPass.SetInt("gPosition", 0);
	shaderLightingPass.SetInt("gNormal", 1);
	shaderLightingPass.SetInt("gAlbedo", 2);
	shaderLightingPass.SetInt("ssao", 3);

	shaderSSAO.Use();
	shaderSSAO.SetInt("gPosition", 0);
	shaderSSAO.SetInt("gNormal", 1);
	shaderSSAO.SetInt("texNoise", 2);

	shaderSSAOBlur.Use();
	shaderSSAOBlur.SetInt("ssaoInput", 0);
}

void SSAOState::HandleInput(SDL_Event* event)
{
	DefaultState::HandleInput(event);
}

void SSAOState::Update(float deltaTime)
{
	DefaultState::Update(deltaTime);
}

void SSAOState::Render(float alpha)
{
	// configure transformation matrices
	glm::mat4 view = m_sceneCamera->GetCamera().GetView();
	glm::mat4 projection = m_sceneCamera->GetCamera().GetProjection();
	glm::vec3 cameraPosition = m_sceneCamera->GetCamera().GetPosition();
	
	// render
	// ------
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 1. geometry pass: render scene's geometry/color data into gbuffer
	// -----------------------------------------------------------------
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderGeometryPass.Use();
		shaderGeometryPass.SetMat4("projection", projection);
		shaderGeometryPass.SetMat4("view", view);

		for (unsigned int i = 0; i < objectPositions.size(); i++)
		{
			ent.GetTransform().SetPosition(objectPositions[i]);
			shaderGeometryPass.SetMat4("model", ent.GetTransform().GetModelMat());
			ent.GetModel().Draw(shaderGeometryPass);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	if (enableSSAO) 
	{
		ssaoFx.Process(projection, gPosition, gNormal);
	}
	// 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
	// -----------------------------------------------------------------------------------------------------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderLightingPass.Use();

	// send light relevant uniforms
	shaderLightingPass.SetVec3("light.Position", glm::vec3(0.2, 0.2, 0.7));
	shaderLightingPass.SetVec3("light.Color", glm::vec3(0.2, 0.2, 0.7));
	// Update attenuation parameters
	const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
	const float linear = 0.09;
	const float quadratic = 0.032;
	shaderLightingPass.SetFloat("light.Linear", linear);
	shaderLightingPass.SetFloat("light.Quadratic", quadratic);

	shaderLightingPass.SetVec3("viewPos", cameraPosition);

	shaderLightingPass.SetBool("enableSSAO", enableSSAO);
	shaderLightingPass.SetBool("ssaoIntensity", ssaoFx.GetParams().Intensity);

	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gAlbedo);
	
	ssaoFx.BindTextureMaps();
	
	quad.Draw();
}

void SSAOState::RenderUI()
{
	DefaultState::RenderUI();

	ImGui::Begin("SSAO Settings");
	SSAO::Params ssaoParams = ssaoFx.GetParams();

	ImGui::Checkbox("Enable", &enableSSAO);
	ImGui::SliderFloat("Intensity", &ssaoParams.Intensity, 0.0f, 1.0f);
	ImGui::SliderInt("Kernel Size", &ssaoParams.KernelSize, 1, 512);
	ImGui::SliderFloat("Kernel Radius", &ssaoParams.Radius, 0, 5);
	ImGui::SliderFloat("Bias", &ssaoParams.Bias, 0.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("Show Buffers");

	if (ssaoFx.GetParams() != ssaoParams)
	{
		ssaoFx.SetParams(ssaoParams);
	}

	const float screenRatio = m_windowParams.Height / static_cast<float>(m_windowParams.Width);
	
	ImGui::Columns(2, NULL, false);

	const float width = ImGui::GetColumnWidth();
	const float height = width * screenRatio;

	ImGui::Text("Position"); ImGui::NextColumn();
	ImGui::Text("Normals"); ImGui::NextColumn();
	ImGui::Image((void*)(intptr_t)gPosition, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0)); ImGui::NextColumn();
	ImGui::Image((void*)(intptr_t)gNormal, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0)); ImGui::NextColumn();

	ImGui::Text("Color"); ImGui::NextColumn();
	ImGui::Text("SSAO"); ImGui::NextColumn();
	ImGui::Image((void*)(intptr_t)gAlbedo, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0)); ImGui::NextColumn();
	ImGui::Image((void*)(intptr_t)ssaoFx.GetColorBufferBlur(), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0)); ImGui::NextColumn();
	
	ImGui::End();
}

void SSAOState::Cleanup()
{

}
