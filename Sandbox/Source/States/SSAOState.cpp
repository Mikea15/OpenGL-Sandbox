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
	//ent.SetModel(m_assetManager->LoadModel("Data/Objects/sponza/sponza.obj"));
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

	// also create framebuffer to hold SSAO processing stage 
	// -----------------------------------------------------
	glGenFramebuffers(1, &ssaoFBO);
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;

	// and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// generate sample kernel
	// ----------------------
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0;

		// scale samples s.t. they're more aligned to center of kernel
		scale = MathUtils::Lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// generate noise texture
	// ----------------------
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}

	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
	glm::mat4 view = m_sceneCameraComp->GetCamera().GetView();
	glm::mat4 projection = m_sceneCameraComp->GetCamera().GetProjection();
	glm::vec3 cameraPosition = m_sceneCameraComp->GetCamera().GetPosition();
	
	// render
	// ------
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
		// 1.1. generate SSAO texture
		// ------------------------
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			shaderSSAO.Use();

			shaderSSAO.SetInt("kernelSize", kernelSize);
			shaderSSAO.SetFloat("radius", radius);
			shaderSSAO.SetFloat("bias", bias);

			// Send kernel + rotation 
			for (unsigned int i = 0; i < 64; ++i) {
				shaderSSAO.SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
			}
			shaderSSAO.SetMat4("projection", projection);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);
			quad.Draw();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// 1.2. blur SSAO texture to remove noise
		// ------------------------------------
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			shaderSSAOBlur.Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
			quad.Draw();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
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
	shaderLightingPass.SetBool("ssaoIntensity", ssaoIntensity);

	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	quad.Draw();

}

void SSAOState::RenderUI()
{
	DefaultState::RenderUI();

	ImGui::Begin("SSAO Settings");
	ImGui::Checkbox("Enable", &enableSSAO);
	ImGui::SliderFloat("Intensity", &ssaoIntensity, 0.0f, 1.0f);
	ImGui::SliderInt("Kernel Size", &kernelSize, 1, 512);
	ImGui::SliderFloat("Kernel Radius", &radius, 0, 5);
	ImGui::SliderFloat("Bias", &bias, 0.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("Show Buffers");

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
	ImGui::Image((void*)(intptr_t)ssaoColorBufferBlur, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0)); ImGui::NextColumn();
	
	ImGui::End();
}

void SSAOState::Cleanup()
{

}
