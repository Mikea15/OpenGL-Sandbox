#include "AdvancedLighting.h"

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>


#include "Systems/Rendering/Primitives.h"
#include "Systems/Rendering/TextureBuffer.h"

#include "Game.h"

#include "Core/Utils.h"

AdvancedLighting::AdvancedLighting()
{
	m_shaders = ShaderManager();

	shader = m_shaders.LoadShader("shader1", "advancedLighting.vs", "gammaCorrection.fs");
	red = m_shaders.LoadShader("redColor", "lamp.vs.glsl", "red.fs");
	green = m_shaders.LoadShader("greenColor", "lamp.vs.glsl", "green.fs");
	blue = m_shaders.LoadShader("blueColor", "lamp.vs.glsl", "blue.fs");

	gridShader = m_shaders.LoadShader("blueColor", "lamp.vs.glsl", "blue.fs");
	depthShader = m_shaders.LoadShader("depth", "simpleDepthShader.vs", "empty.fs");
	modelShader = m_shaders.LoadShader("yellow", "standard.vs", "yellow.fs");
}

void AdvancedLighting::Init(Game* game)
{
	m_windowParams = game->GetWindowParameters();
	m_sdlHandler = game->GetSDLHandler();

	m_sceneCameraComp = &game->GetSystemComponentManager()->GetComponent<SceneCameraComponent>();
	AssetManager& assetManager = game->GetAssetManager();

	floorTexture = assetManager.LoadTexture("Data/Images/wood.png", false).id;
	
	TextureBuffer::CreateDepthMapFBO(m_windowParams.Width, m_windowParams.Height, depthMapFBO, depthMap);

	modelShader.Use();
	modelShader.SetInt("diffuseTexture", 0);
	modelShader.SetInt("shadowMap", 1);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void AdvancedLighting::HandleInput(SDL_Event* event)
{
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_b)
	{
		isBlinn = !isBlinn;
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_g)
	{
		isGammaCorrected = !isGammaCorrected;
	}
}

void AdvancedLighting::Update(float deltaTime)
{
	
}

void AdvancedLighting::Render(float alpha)
{
	glm::mat4 projection = m_sceneCameraComp->GetCamera().ProjectionMatrix();
	glm::mat4 view = m_sceneCameraComp->GetCamera().View();

	lightPosition = glm::vec3(-2.0f, 4.0f, -1.0f);
	// 1. render depth of scene to texture (from light's perspective)
	// --------------------------------------------------------------
	glm::mat4 lightProjection = glm::mat4(1.0f);
	glm::mat4 lightView = glm::mat4(1.0f);
	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	depthShader.Use();
	depthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	glCullFace(GL_FRONT);
	{
		// floor
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));
		modelShader.SetMat4("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// cubes
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));

		depthShader.SetMat4("model", model);
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
		model = glm::scale(model, glm::vec3(0.5f));

		depthShader.SetMat4("model", model);
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
		model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(0.25));

		depthShader.SetMat4("model", model);
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. render scene as normal using the generated depth/shadow map  
	// --------------------------------------------------------------
	glViewport(0, 0, m_windowParams.Width, m_windowParams.Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelShader.Use();
	modelShader.SetMat4("projection", projection);
	modelShader.SetMat4("view", view);
	// set light uniforms
	modelShader.SetVec3("viewPos", m_sceneCameraComp->GetCamera().GetPosition());
	modelShader.SetVec3("lightPos", lightPosition);
	modelShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	{
		//----
		// floor
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));
		modelShader.SetMat4("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// cubes
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));

		modelShader.SetMat4("model", model);
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
		model = glm::scale(model, glm::vec3(0.5f));

		modelShader.SetMat4("model", model);
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
		model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(0.25));

		modelShader.SetMat4("model", model);
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		//----
	}
}

void AdvancedLighting::RenderUI()
{
	
}

void AdvancedLighting::Cleanup()
{
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
}
