#include "BloomEffectState.h"

#include "Game.h"
#include "Core/Utils.h"
#include "Systems/Rendering/Primitives.h"

void BloomEffectState::Init(Game* game)
{
	m_windowParams = game->GetWindowParameters();
	m_sdlHandler = game->GetSDLHandler();

	m_sceneCameraComp = &game->GetSystemComponentManager()->GetComponent<SceneCameraComponent>();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	shader = shaderManager.LoadShader("bloom", "bloom.vs", "bloom.fs");
	shaderLight = shaderManager.LoadShader("bloomLight", "bloom.vs", "light_box.fs");
	shaderBlur = shaderManager.LoadShader("bloomBlur", "blur.vs", "blur.fs");
	shaderBloomFinal = shaderManager.LoadShader("bloomFinal", "bloom_final.vs", "bloom_final.fs");
	skyboxShader = shaderManager.LoadShader("gradientSkybox", "skybox.vs", "skybox/horizon_sun.fs");

	// load textures
	// -------------
	
	woodTexture = assetManager.LoadTexture("Data/Images/wood.png", true).id;
	containerTexture = assetManager.LoadTexture("Data/Images/container.jpg", true).id;

	// configure (floating point) framebuffers
	// ---------------------------------------
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}

	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_windowParams.Width, m_windowParams.Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// ping-pong-framebuffer for blurring
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// lighting info
	// -------------
	// positions
	lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
	lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
	lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
	lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
	
	// colors
	lightColors.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	lightColors.push_back(glm::vec3(1.5f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 1.5f));
	lightColors.push_back(glm::vec3(0.0f, 1.5f, 0.0f));

	// shader configuration
	// --------------------
	shader.Use();
	shader.SetInt("diffuseTexture", 0);
	shaderBlur.Use();
	shaderBlur.SetInt("image", 0);
	shaderBlur.SetVec2("resolution", glm::vec2(m_windowParams.Width, m_windowParams.Height));
	shaderBloomFinal.Use();
	shaderBloomFinal.SetInt("scene", 0);
	shaderBloomFinal.SetInt("bloomBlur", 1);
	// skyboxShader.Use();
	// skyboxShader.SetInt("skybox", 0);

	cube = Entity();
	assetManager.LoadModel("Data/Objects/default/cube1m.fbx", cube.GetModel());

	skybox = Skybox();
}

void BloomEffectState::HandleInput(SDL_Event * event)
{
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_b) bloom = !bloom;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_u) exposure += 0.05f;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_j) {
		if (exposure > 0.0f)
			exposure -= 0.05f;
		if (exposure <= 0.0f)
			exposure = 0.0f;
	}
}

void BloomEffectState::Update(float deltaTime)
{
	
}

void BloomEffectState::Render(float alpha)
{
	// render
	// ------
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 1. render scene into floating point framebuffer
	// -----------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	
	glViewport(0, 0, m_windowParams.Width * renderScale, m_windowParams.Height * renderScale);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = m_sceneCameraComp->GetCamera().ProjectionMatrix();
	glm::mat4 view = m_sceneCameraComp->GetCamera().View();

	shader.Use();
	shader.SetMat4("projection", projection);
	shader.SetMat4("view", view);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	// set lighting uniforms
	for (unsigned int i = 0; i < lightPositions.size()-1; i++)
	{
		shader.SetVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
		shader.SetVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
	}

	shader.SetVec3("lights[3].Position", m_sceneCameraComp->GetCamera().GetPosition());
	shader.SetVec3("lights[3].Color", glm::vec3(1.0f));

	shader.SetVec3("viewPos", m_sceneCameraComp->GetCamera().GetPosition());

	// create one large cube that acts as the floor
	cube.GetTransform().SetPosition(glm::vec3(0.0f, -1.0f, 0.0));
	cube.GetTransform().SetScale(glm::vec3(12.5f, 0.5f, 12.5f));
	shader.SetMat4("model", cube.GetTransform().GetModelMat());
	cube.GetModel().Draw(shader);

	// then create multiple cubes as the scenery
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	cube.GetTransform().SetPosition(glm::vec3(0.0f, 1.5f, 0.0));
	cube.GetTransform().SetScale(glm::vec3(0.5f));
	shader.SetMat4("model", cube.GetTransform().GetModelMat());
	cube.GetModel().Draw(shader);

	cube.GetTransform().SetPosition(glm::vec3(2.0f, 0.0f, 1.0));
	cube.GetTransform().SetScale(glm::vec3(0.5f));
	shader.SetMat4("model", cube.GetTransform().GetModelMat());
	cube.GetModel().Draw(shader);

	trans[0] = Transform();
	trans[0].SetPosition(glm::vec3(-1.0f, -1.0f, 2.0));
	trans[0].RotateLocal(glm::normalize(glm::vec3(1.0, 0.0, 1.0)), 60.0f);
	shader.SetMat4("model", trans[0].GetModelMat());
	cube.GetModel().Draw(shader);

	trans[1] = Transform();
	trans[1].SetPosition(glm::vec3(0.0f, 2.7f, 4.0));
	trans[1].RotateLocal(glm::normalize(glm::vec3(1.0, 0.0, 1.0)), 23.0f);
	trans[1].SetScale(glm::vec3(1.25));
	shader.SetMat4("model", trans[1].GetModelMat());
	cube.GetModel().Draw(shader);

	trans[2] = Transform();
	trans[2].SetPosition(glm::vec3(-2.0f, 1.0f, -3.0));
	trans[2].RotateLocal(glm::normalize(glm::vec3(1.0, 0.0, 1.0)), 124.0f);
	shader.SetMat4("model", trans[2].GetModelMat());
	cube.GetModel().Draw(shader);

	cube.GetTransform().SetPosition(glm::vec3(-3.0f, 0.0f, 0.0));
	cube.GetTransform().SetScale(glm::vec3(0.5f));
	shader.SetMat4("model", cube.GetTransform().GetModelMat());
	cube.GetModel().Draw(shader);

	// finally show all the light sources as bright cubes
	shaderLight.Use();
	shaderLight.SetMat4("projection", projection);
	shaderLight.SetMat4("view", view);

	Transform tmp = Transform();
	tmp.Scale(0.25f);
	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
		tmp.SetPosition(lightPositions[i]);
		shaderLight.SetMat4("model", tmp.GetModelMat());
		shaderLight.SetVec3("lightColor", lightColors[i]);
		cube.GetModel().Draw(shaderLight);
	}

	// render skybox last. but before transparent objects
	skyboxShader.Use();
	skyboxShader.SetMat4("projection", projection);
	skyboxShader.SetMat4("view", view);
	skyboxShader.SetVec3("TopColor", topColor);
	skyboxShader.SetFloat("TopExponent", topExp);
	skyboxShader.SetVec3("HorizonColor", horizonColor);
	skyboxShader.SetVec3("BottomColor", bottomColor);
	skyboxShader.SetFloat("BottomExponent", bottomExp);
	skyboxShader.SetFloat("SkyIntensity", skyIntensity);
	skyboxShader.SetVec3("SunColor", sunColor);
	skyboxShader.SetFloat("SunIntensity", sunIntensity);
	skyboxShader.SetFloat("SunAlpha", sunAlpha);
	skyboxShader.SetFloat("SunBeta", sunBeta);
	//skyboxShader.SetVec3("SunDirection", sunDirection);
	skyboxShader.SetFloat("SunAzimuth", sunAzimuth);
	skyboxShader.SetFloat("SunAltitude", sunAltitude);

	skybox.Draw(skyboxShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 2. blur bright fragments with two-pass Gaussian Blur 
	// --------------------------------------------------
	bool horizontal = true, first_iteration = true;
	shaderBlur.Use();
	for (unsigned int i = 0; i < blurPasses; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		shaderBlur.SetInt("horizontal", horizontal);
		shaderBlur.SetInt("gauss", useBetterGauss);
		shaderBlur.SetFloat("radius", blurRadius);

		glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		
		quad.Draw();

		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
	// --------------------------------------------------------------------------------------------------------------------------
	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderBloomFinal.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[renderBlurPass?1:0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
	shaderBloomFinal.SetInt("hdr", hdr);
	shaderBloomFinal.SetInt("bloom", bloom);
	shaderBloomFinal.SetFloat("exposure", exposure);
	shaderBloomFinal.SetFloat("renderScale", renderScale);
	
	quad.Draw();
}

void BloomEffectState::RenderUI()
{
	ImGui::Begin("Rendering Settings"); ImGui::BeginGroup();

	ImGui::Checkbox("HDR", &hdr); ImGui::SameLine();
	ImGui::Checkbox("Bloom", &bloom);
	ImGui::SliderFloat("Exposure", &exposure, 0.0f, 5.0f);
	ImGui::Checkbox("Render Blur Pass", &renderBlurPass);
	ImGui::Checkbox("Better Blur (Gauss)", &useBetterGauss);
	ImGui::SliderFloat("Blur Radius", &blurRadius, 0.0f, 10.0f);
	ImGui::SliderInt("Blur Passes", &blurPasses, 0, 40);
	ImGui::SliderFloat("Render Scale", &renderScale, 0.1f, 2.0f);

	ImGui::EndGroup(); ImGui::End();

	ImGui::Begin("Skybox Settings"); ImGui::BeginGroup();

	auto colorSliderUpdate = [&](const std::string& name, glm::vec3& outColor)
	{
		float color[3] = { outColor.x, outColor.y, outColor.z };
		ImGui::ColorEdit3(name.c_str(), color, ImGuiColorEditFlags_RGB);
		outColor = glm::vec3(color[0], color[1], color[2]);
	};

	
	colorSliderUpdate("Top Color", topColor);
	ImGui::SliderFloat("Top Exponent", &topExp, 0.0f, 100.0f);
	ImGui::Separator();
	colorSliderUpdate("Horizon Color", horizonColor);
	ImGui::Separator();
	colorSliderUpdate("Bottom Color", bottomColor);
	ImGui::SliderFloat("Bottom Exponent", &bottomExp, 0.0f, 100.0f);
	ImGui::Separator();
	ImGui::SliderFloat("Sky Intensity", &skyIntensity, 0.0f, 2.0f);
	colorSliderUpdate("Sun Color", sunColor);
	ImGui::Separator();
	ImGui::SliderFloat("Sun Intensity", &sunIntensity, 0.0f, 3.0f);
	ImGui::SliderFloat("Sun Alpha", &sunAlpha, 0.0f, 1000.0f);
	ImGui::SliderFloat("Sun Beta", &sunBeta, 0.0f, 1.0f);
	ImGui::SliderFloat("Sun Azimuth (deg)", &sunAzimuth, 0.0f, 360.0f);
	ImGui::SliderFloat("Sun Altitude (deg)", &sunAltitude, 0.0f, 360.0f);


	ImGui::EndGroup(); ImGui::End();
}

void BloomEffectState::Cleanup()
{
}
