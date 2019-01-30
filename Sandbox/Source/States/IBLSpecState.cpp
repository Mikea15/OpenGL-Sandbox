#include "IBLSpecState.h"

#include "Game.h"

#include <random>

static float ccCamNearPlane = 1.0f;
static float ccCamFarPlane = 200.0f;
static float ccCamFov = 60.0f;
static float ccRotationSpeed = 10.0f;

IBLSpecState::IBLSpecState()
{

}

IBLSpecState::~IBLSpecState()
{

}

void IBLSpecState::Init(Game* game)
{
	DefaultState::Init(game);

	// set depth function to less than AND equal for skybox depth trick.
	glDepthFunc(GL_LEQUAL);
	// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	m_qTree = QuadTree(glm::vec3(0.0f), 50.0f);
	m_oTree = Octree(glm::vec3(0.0f), 50.0f);

	debugInstanced = shaderManager.LoadShader("debugInstanced", "instancing.vs", "unlit/color.fs");
	pbrShader = shaderManager.LoadShader("pbrShader", "pbr/pbr.vs", "pbr/pbr.fs");
	equirectangularToCubemapShader = shaderManager.LoadShader("equirectangularToCubemapShader", "pbr/cubemap.vs", "pbr/equirectangular_to_cubemap.fs");
	irradianceShader = shaderManager.LoadShader("irradianceShader", "pbr/cubemap.vs", "pbr/irradiance_convolution.fs");
	prefilterShader = shaderManager.LoadShader("prefilterShader", "pbr/cubemap.vs", "pbr/prefilter.fs");
	brdfShader = shaderManager.LoadShader("brdfShader", "pbr/brdf.vs", "pbr/brdf.fs");
	backgroundShader = shaderManager.LoadShader("backgroundShader", "pbr/background.vs", "pbr/background.fs");

	wireframeShader = shaderManager.LoadShader("wireframeSimple", "unlit/simple.vs", "unlit/color.fs");
	wireframeShader.Use();
	wireframeShader.SetVec3("Color", glm::vec3(0, 0, 1.0));

	pbrShader.Use();
	pbrShader.SetInt("irradianceMap", 0);
	pbrShader.SetInt("prefilterMap", 1);
	pbrShader.SetInt("brdfLUT", 2);
	pbrShader.SetInt("albedoMap", 3);
	pbrShader.SetInt("normalMap", 4);
	pbrShader.SetInt("metallicMap", 5);
	pbrShader.SetInt("roughnessMap", 6);
	pbrShader.SetInt("aoMap", 7);
	pbrShader.SetInt("heightMap", 8);

	backgroundShader.Use();
	backgroundShader.SetInt("environmentMap", 0);

	// load PBR material textures
	// --------------------------
	// rusted iron
	m_assetManager->LoadTextureAsync("Data/Images/pbr/rusted_iron/albedo.png", false, &ironAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/rusted_iron/normal.png", false, &ironNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/rusted_iron/metallic.png", false, &ironMetallicMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/rusted_iron/roughness.png", false, &ironRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/rusted_iron/ao.png", false, &ironAOMap);

	// gold
	m_assetManager->LoadTextureAsync("Data/Images/pbr/gold/albedo.png", false, &goldAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/gold/normal.png", false, &goldNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/gold/metallic.png", false, &goldMetallicMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/gold/roughness.png", false, &goldRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/gold/ao.png", false, &goldAOMap);

	// grass
	m_assetManager->LoadTextureAsync("Data/Images/pbr/grass/albedo.png", false, &grassAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/grass/normal.png", false, &grassNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/grass/metallic.png", false, &grassMetallicMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/grass/roughness.png", false, &grassRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/grass/ao.png", false, &grassAOMap);

	// plastic
	m_assetManager->LoadTextureAsync("Data/Images/pbr/plastic/albedo.png", false, &plasticAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/plastic/normal.png", false, &plasticNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/plastic/metallic.png", false, &plasticMetallicMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/plastic/roughness.png", false, &plasticRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/plastic/ao.png", false, &plasticAOMap);

	// wall
	m_assetManager->LoadTextureAsync("Data/Images/pbr/wall/albedo.png", false, &wallAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/wall/normal.png", false, &wallNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/wall/metallic.png", false, &wallMetallicMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/wall/roughness.png", false, &wallRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/wall/ao.png", false, &wallAOMap);

	// marble
	m_assetManager->LoadTextureAsync("Data/Images/pbr/marble/albedo.png", false, &marbleAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/marble/normal.png", false, &marbleNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/marble/height.png", false, &marbleHeightMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/marble/roughness.png", false, &marbleRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/marble/ao.png", false, &marbleAOMap);

	// granite
	m_assetManager->LoadTextureAsync("Data/Images/pbr/cliffgranite/albedo.png", false, &graniteAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/cliffgranite/normal.png", false, &graniteNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/cliffgranite/height.png", false, &graniteHeightMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/cliffgranite/roughness.png", false, &graniteRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/cliffgranite/ao.png", false, &graniteAOMap);

	// leather
	m_assetManager->LoadTextureAsync("Data/Images/pbr/leather/albedo.png", false, &leatherAlbedoMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/leather/normal.png", false, &leatherNormalMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/leather/height.png", false, &leatherHeightMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/leather/roughness.png", false, &leatherRoughnessMap);
	m_assetManager->LoadTextureAsync("Data/Images/pbr/leather/ao.png", false, &leatherAOMap);

	lightPositions.push_back(glm::vec3(-10.0f, 10.0f, 10.0f));
	lightPositions.push_back(glm::vec3(10.0f, 10.0f, 10.0f));
	lightPositions.push_back(glm::vec3(-10.0f, -10.0f, 10.0f));
	lightPositions.push_back(glm::vec3(10.0f, -10.0f, 10.0f));

	lightColors.push_back(glm::vec3(300.0f, 300.0f, 300.0f));
	lightColors.push_back(glm::vec3(300.0f, 300.0f, 300.0f));
	lightColors.push_back(glm::vec3(300.0f, 300.0f, 300.0f));
	lightColors.push_back(glm::vec3(300.0f, 300.0f, 300.0f));

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	const int fboResolution = 2048;

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fboResolution, fboResolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	hdrTexture = m_assetManager->LoadHDRTexure("Data/Images/pbr/newport_loft.hdr");

	// pbr: setup cubemap to render to and attach to framebuffer
	// ---------------------------------------------------------
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, fboResolution, fboResolution, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	captureViews.push_back(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	captureViews.push_back(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	captureViews.push_back(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	captureViews.push_back(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	captureViews.push_back(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	captureViews.push_back(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	// pbr: convert HDR equirectangular environment map to cubemap equivalent
	// ----------------------------------------------------------------------
	equirectangularToCubemapShader.Use();
	equirectangularToCubemapShader.SetInt("equirectangularMap", 0);
	equirectangularToCubemapShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, fboResolution, fboResolution); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Primitives::RenderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	irradianceShader.Use();
	irradianceShader.SetInt("environmentMap", 0);
	irradianceShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Primitives::RenderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	// --------------------------------------------------------------------------------
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	prefilterShader.Use();
	prefilterShader.SetInt("environmentMap", 0);
	prefilterShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Primitives::RenderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
	glGenTextures(1, &brdfLUTTexture);
	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, fboResolution, fboResolution, 0, GL_RG, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fboResolution, fboResolution);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, fboResolution, fboResolution);
	brdfShader.Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Primitives::RenderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// initialize static shader uniforms before rendering
	// --------------------------------------------------
	glm::mat4 projection = m_sceneCameraComp->GetCamera().GetProjection();

	pbrShader.Use();
	pbrShader.SetMat4("projection", projection);
	backgroundShader.Use();
	backgroundShader.SetMat4("projection", projection);

	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);

	// create a 3d grid of cubes.
	gX = gY = gZ = 10;
	gY = 1;

	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(-20, 20); // define the range

	for (int gXStart = -gX; gXStart < gX; gXStart++)
	{
		for (int gYStart = -gY; gYStart < gY; gYStart++)
		{
			for (int gZStart = -gZ; gZStart < gZ; gZStart++)
			{
				glm::vec3 v3 = glm::vec3(distr(eng), gY, distr(eng)) * gridSpacing;
				positions.push_back(v3);
				visible.push_back(ContainmentType::Disjoint);

				m_qTree.Insert(v3);
				m_oTree.Insert(v3);
			}
		}
	}

	// configure (floating point) framebuffers
	// ---------------------------------------
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_windowParams.Width, m_windowParams.Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);


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

	topDownCamera.SetFov(m_sceneCameraComp->GetCamera().GetFov());
	topDownCamera.SetNearFarPlane(
		m_sceneCameraComp->GetCamera().GetNearPlane(),
		m_sceneCameraComp->GetCamera().GetFarPlane()
	);

	
	cc.SetNearFarPlane(ccCamNearPlane, ccCamFarPlane);
	cc.SetFov(ccCamFov);
}

void IBLSpecState::HandleInput(SDL_Event* event)
{
	DefaultState::HandleInput(event);
}

void IBLSpecState::Update(float deltaTime)
{
	DefaultState::Update(deltaTime);

	glm::vec3 camPos = m_sceneCameraComp->GetCamera().GetPosition();
	glm::vec3 camPosOverHead = camPos + glm::vec3(0, 1, 0) * 3.0f;

	if (includeCamPosIntoTrees)
	{
		camPushPosTime += deltaTime;
		if (camPushPosTime > includeFreq)
		{
			glm::vec3 positionToAdd = camPos + m_sceneCameraComp->GetCamera().GetForward() * 2.0f;
			positions.push_back(positionToAdd);
			m_qTree.Insert(positionToAdd);
			m_oTree.Insert(positionToAdd);
			visible.push_back(ContainmentType::Disjoint);
			camPushPosTime = 0.0f;
		}
	}

	cc.UpdateLookAt(glm::vec2(1.0f, 0.0f) * ccRotationSpeed * deltaTime);
	cc.Update(deltaTime);

	topDownCamera.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
	topDownCamera.Update(deltaTime);

	rotatingT.RotateLocal(glm::vec3(0, 1, 0), 15.0f * deltaTime);
}

void IBLSpecState::Render(float alpha)
{
	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);

	vertexCountStats = 0;

	// configure transformation matrices
	glm::mat4 view = m_sceneCameraComp->GetCamera().GetView();
	glm::mat4 projection = m_sceneCameraComp->GetCamera().GetProjection();
	glm::vec3 cameraPosition = m_sceneCameraComp->GetCamera().GetPosition();

	// render
	// ------
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bool showLines = true;
	if (showLines)
	{
		wireframeShader.Use();
		wireframeShader.SetMat4("view", view);
		wireframeShader.SetMat4("projection", projection);
		wireframeShader.SetVec3("Color", glm::vec3(0.1, 0.8, 0.2));

		Transform t;
		
		t.SetPosition(glm::vec3(0.0f));
		wireframeShader.SetVec3("Color", glm::vec3(0.0f, 0.0f, 0.0f));
		wireframeShader.SetMat4("model", t.GetModelMat());
		Primitives::RenderPoint(10);

		t.SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));
		wireframeShader.SetVec3("Color", glm::vec3(0.0f, 1.0f, 0.0f));
		wireframeShader.SetMat4("model", t.GetModelMat());
		Primitives::RenderPoint(10);

		t.SetPosition(glm::vec3(0.0f));
		wireframeShader.SetMat4("model", t.GetModelMat());
		Primitives::RenderLine(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		t.SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
		wireframeShader.SetVec3("Color", glm::vec3(1.0f, 0.0f, 0.0f));
		wireframeShader.SetMat4("model", t.GetModelMat());
		Primitives::RenderPoint(10);

		t.SetPosition(glm::vec3(0.0f));
		wireframeShader.SetMat4("model", t.GetModelMat());
		Primitives::RenderLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		t.SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		wireframeShader.SetVec3("Color", glm::vec3(0.0f, 0.0f, 1.0f));
		wireframeShader.SetMat4("model", t.GetModelMat());
		Primitives::RenderPoint(10);

		wireframeShader.SetMat4("model", t.GetModelMat());
		
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

		Primitives::RenderLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	bool showSpheresTextured = true;
	if (showSpheresTextured)
	{
		// render scene, supplying the convoluted irradiance map to the final shader.
		// ------------------------------------------------------------------------------------------
		pbrShader.Use();
		pbrShader.SetMat4("view", view);
		pbrShader.SetMat4("projection", projection);
		pbrShader.SetVec3("camPos", cameraPosition);
		pbrShader.SetFloat("heightScale", 0.1f);

		// bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		// rusted iron
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, ironNormalMap);
		glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, ironAOMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(-5.0, 0.0, 2.0));
			t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// gold
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, goldNormalMap);
		glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, goldAOMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(-3.0, 0.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// grass
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, grassNormalMap);
		glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, grassAOMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(-1.0, 0.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// plastic
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
		glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, plasticAOMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(1.0, 0.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// wall
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, wallNormalMap);
		glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, wallAOMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(3.0, 0.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// marble
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, marbleAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, marbleNormalMap);

		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, marbleRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, marbleAOMap);
		glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, marbleHeightMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(-1.0, 2.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// granite
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, graniteAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, graniteNormalMap);

		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, graniteRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, graniteAOMap);
		glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, graniteHeightMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(1.0, 2.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		// leather
		glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, leatherAlbedoMap);
		glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, leatherNormalMap);

		glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, leatherRoughnessMap);
		glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, leatherAOMap);
		glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, leatherHeightMap);
		{
			Transform t;
			t.SetPosition(glm::vec3(3.0, 2.0, 2.0)); t.SetOrientation(rotatingT.GetOrientation());
			pbrShader.SetMat4("model", t.GetModelMat());
			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}
	}

	bool showLights = false;
	if (showLights)
	{

		// render light source (simply re-render sphere at light positions)
		// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
		// keeps the codeprint small.
		for (unsigned int i = 0; i < lightPositions.size(); ++i)
		{
			Transform lightTransform;
			lightTransform.SetPosition(lightPositions[i]);

			pbrShader.SetMat4("model", lightTransform.GetModelMat());
			pbrShader.SetVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
			pbrShader.SetVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

			Primitives::RenderSphere();
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}
	}

	bool showSpeheres = true;
	if (showSpeheres)
	{
		// draw 3d grid of cubes.
		const unsigned int size = positions.size();
		for (unsigned int i = 0; i < size; ++i)
		{
			scratchTransform.SetPosition(positions[i]);
			pbrShader.SetMat4("model", scratchTransform.GetModelMat());

			visible[i] = cc.GetBoundingFrustum().Contains(BoundingBox(positions[i], 1.0f));
			if (visible[i] == ContainmentType::Disjoint)
			{
				wireframeShader.Use();
				wireframeShader.SetMat4("view", view);
				wireframeShader.SetMat4("projection", projection);
				wireframeShader.SetVec3("camPos", cameraPosition);
				wireframeShader.SetMat4("model", scratchTransform.GetModelMat());
				wireframeShader.SetVec3("Color", glm::vec3(0.1f, 0.2f, 0.5f));

				Primitives::RenderSphere(true);
			}
			else if (visible[i] == ContainmentType::Intersects)
			{
				wireframeShader.Use();
				wireframeShader.SetMat4("view", view);
				wireframeShader.SetMat4("projection", projection);
				wireframeShader.SetVec3("camPos", cameraPosition);
				wireframeShader.SetMat4("model", scratchTransform.GetModelMat());
				wireframeShader.SetVec3("Color", glm::vec3(1.0f, 1.0f, 1.0f));
				Primitives::RenderSphere(true);
			}
			else
			{
				pbrShader.Use();
				pbrShader.SetMat4("view", view);
				pbrShader.SetMat4("projection", projection);
				pbrShader.SetVec3("camPos", cameraPosition);

				pbrShader.SetMat4("model", scratchTransform.GetModelMat());
				Primitives::RenderSphere(false);
			}
			
			
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}
	}

	if (showQtree)
	{
		std::vector<Rect> quadTreeVis;
		m_qTree.GetAllBoundingBoxes(quadTreeVis);
		const unsigned int qSize = quadTreeVis.size();
		for (unsigned int i = 0; i < qSize; ++i)
		{
			Transform origin;
			auto pos2D = quadTreeVis[i].GetPosition();

			origin.SetPosition(glm::vec3(pos2D.x, 0.0f, pos2D.y));
			origin.RotateLocal(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
			origin.SetScale(glm::vec3(quadTreeVis[i].GetSize()));

			wireframeShader.Use();
			wireframeShader.SetMat4("view", view);
			wireframeShader.SetMat4("projection", projection);
			wireframeShader.SetVec3("Color", glm::vec3(0.1, 0.8, 0.2));
			wireframeShader.SetMat4("model", origin.GetModelMat());
			Primitives::RenderQuad(true);
			vertexCountStats += Primitives::quad.GetVertexCount();
		}
	}

	if (showOctree)
	{
		std::vector<BoundingBox> ocTreeVis;
		m_oTree.GetAllBoundingBoxes(ocTreeVis);
		const unsigned int qSize = ocTreeVis.size();
		for (unsigned int i = 0; i < qSize; ++i)
		{
			Transform origin;
			auto pos = ocTreeVis[i].GetPosition();

			origin.SetPosition(pos);
			origin.SetScale(glm::vec3(ocTreeVis[i].GetSize()));

			wireframeShader.Use();
			wireframeShader.SetMat4("view", view);
			wireframeShader.SetMat4("projection", projection);
			wireframeShader.SetVec3("Color", glm::vec3(0.1, 0.8, 0.2));
			wireframeShader.SetMat4("model", origin.GetModelMat());
			Primitives::RenderCube(true);
			vertexCountStats += Primitives::cube.GetVertexCount();
		}
	}

	bool drawSkybox = true;
	if (drawSkybox) 
	{
		// render skybox (render as last to prevent overdraw)
		backgroundShader.Use();
		backgroundShader.SetMat4("projection", projection);
		backgroundShader.SetMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		// glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		// glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
		Primitives::RenderCube();
		vertexCountStats += Primitives::cube.GetVertexCount();
	}
	// render BRDF map to screen
	// brdfShader.Use();
	// Primitives::RenderQuad();

	// render top down view in wireframe
	bool renderTopDownBuffer = true;
	if( renderTopDownBuffer )
	{
		// 1. render scene into floating point framebuffer
		// -----------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// configure transformation matrices
		glm::mat4 view = topDownCamera.GetView();
		glm::mat4 projection = topDownCamera.GetProjection();
		glm::vec3 cameraPosition = topDownCamera.GetPosition();
		glm::mat4 ortho = topDownCamera.OrthographicMatrix();

		wireframeShader.Use();
		wireframeShader.SetMat4("view", view);
		wireframeShader.SetMat4("projection", ortho);

		// draw 3d grid of cubes.
		const unsigned int size = positions.size();
		for (unsigned int i = 0; i < size; ++i)
		{
			switch (visible[i])
			{
			case ContainmentType::Contains:
				wireframeShader.SetVec3("Color", glm::vec3(0.1f, 0.9f, 0.1f));
				break;
			case ContainmentType::Intersects:
				wireframeShader.SetVec3("Color", glm::vec3(1.0f, 1.0f, 1.0f));
				break;
			case ContainmentType::Disjoint:
				continue;
				wireframeShader.SetVec3("Color", glm::vec3(0.1f, 0.1f, 0.1f));
				break;
			default: break;
			}

			scratchTransform.SetPosition(positions[i]);
			wireframeShader.SetMat4("model", scratchTransform.GetModelMat());

			Primitives::RenderSphere(true);
			vertexCountStats += Primitives::sphere.GetVertexCount();
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void IBLSpecState::RenderUI()
{
	DefaultState::RenderUI();

	ImGui::Begin("Rendering Stats");

	ImGui::Text("Vertex Count: %d", vertexCountStats);

	ImGui::End();

	ImGui::Begin("Camera Q/O Tree Magic");

	ImGui::Checkbox("Insert Camera Positions", &includeCamPosIntoTrees);
	ImGui::SliderFloat("Include Freq (s): %f", &includeFreq, 0.2f, 2.0f);

	ImGui::End();

	ImGui::Begin("Spatial Data Structures");

	ImGui::Checkbox("Show Quadtree", &showQtree);
	ImGui::Checkbox("Show Octree", &showOctree);

	ImGui::End();

	ImGui::Begin("Show Buffers");
	const float screenRatio = m_windowParams.Height / static_cast<float>(m_windowParams.Width);

	ImGui::Columns(1, NULL, false);

	const float width = 250.0f;
	const float height = width * screenRatio;
	ImGui::Image((void*)(intptr_t)colorBuffer, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0)); ImGui::NextColumn();

	ImGui::End();

	ImGui::Begin("CC Settings");

	ImGui::SliderFloat("Rotation Speed", &ccRotationSpeed, 0.0f, 100.0f);
	ImGui::SliderFloat("Near plane", &ccCamNearPlane, 0.1f, ccCamFarPlane);
	ImGui::SliderFloat("Far plane", &ccCamFarPlane, ccCamNearPlane, 1000.0f);
	ImGui::SliderFloat("FOV", &ccCamFov, 0.1f, 179.0f);

	cc.SetFov(ccCamFov);
	cc.SetNearFarPlane(ccCamNearPlane, ccCamFarPlane);

	ImGui::End();
}

void IBLSpecState::Cleanup()
{

}
