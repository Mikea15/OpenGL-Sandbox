#include "MultipleLightsState.h"

#include <algorithm>

#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Game.h"

MultipleLightsState::MultipleLightsState()
{
	m_shaders = ShaderManager();
	lightingShader = m_shaders.LoadShader("lighting2", "multiple_lights.vs.glsl", "multiple_lights.fs.glsl");
	lampShader = m_shaders.LoadShader("lighting3", "lamp.vs.glsl", "lamp.fs.glsl");
	skyboxShader = m_shaders.GetShader("skybox");
}

void MultipleLightsState::Init(Game* game)
{
	m_windowParams = game->GetWindowParameters();
	m_sdlHandler = game->GetSDLHandler();

	m_sceneCameraComp = &game->GetSystemComponentManager()->GetComponent<SceneCameraComponent>();
	AssetManager& assetManager = game->GetAssetManager();

	skyboxShader.Use();
	skyboxShader.SetInt("skybox", 0);

	m_skybox = Skybox();

	// positions all containers
	cubePositions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	cubePositions.push_back(glm::vec3(2.0f, 5.0f, -15.0f));
	cubePositions.push_back(glm::vec3(-1.5f, -2.2f, -2.5f));
	cubePositions.push_back(glm::vec3(-3.8f, -2.0f, -12.3f));
	cubePositions.push_back(glm::vec3(2.4f, -0.4f, -3.5f));
	cubePositions.push_back(glm::vec3(-1.7f, 3.0f, -7.5f));
	cubePositions.push_back(glm::vec3(1.3f, -2.0f, -2.5f));
	cubePositions.push_back(glm::vec3(1.5f, 2.0f, -2.5f));
	cubePositions.push_back(glm::vec3(1.5f, 0.2f, -1.5f));
	cubePositions.push_back(glm::vec3(-1.3f, 1.0f, -1.5f));

	pointLightPositions.push_back(glm::vec3(0.7f, 0.2f, 2.0f));
	pointLightPositions.push_back(glm::vec3(2.3f, -3.3f, -4.0f));
	pointLightPositions.push_back(glm::vec3(-4.0f, 2.0f, -12.0f));
	pointLightPositions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	diffuseMap = assetManager.LoadTexture("Data/Images/container2.png").id;
	specularMap = assetManager.LoadTexture("Data/Images/container2_specular.png").id;

	cubeObject = Entity();
	//cubeObject.SetModel(assetManager.LoadModel("Data/Objects/default/cube1m.fbx"));
	cubeObject.GetTransform().Translate(glm::vec3(-5.0f, 1.0f, 0.0f));

	// shader configuration
	// --------------------
	lightingShader.Use();
	lightingShader.SetInt("material.diffuse", 0);
	lightingShader.SetInt("material.specular", 1);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
}

void MultipleLightsState::HandleInput(SDL_Event* event)
{
	
}

void MultipleLightsState::Update(float deltaTime)
{

}

void MultipleLightsState::Render(float alpha)
{
	glm::mat4 projection = m_sceneCameraComp->GetCamera().ProjectionMatrix();
	glm::mat4 view = m_sceneCameraComp->GetCamera().View();

	// render
	// ------
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// be sure to activate shader when setting uniforms/drawing objects
	lightingShader.Use();
	lightingShader.SetVec3("viewPos", m_sceneCameraComp->GetCamera().GetPosition());
	lightingShader.SetFloat("material.shininess", 32.0f);

	/*
	   Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
	   the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
	   by defining light types as classes and set their values in there, or by using a more efficient uniform approach
	   by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
	*/
	// directional light
	lightingShader.SetVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	lightingShader.SetVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.SetVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	lightingShader.SetVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	lightingShader.SetVec3("pointLights[0].position", pointLightPositions[0]);
	lightingShader.SetVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.SetVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.SetVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.SetFloat("pointLights[0].constant", 1.0f);
	lightingShader.SetFloat("pointLights[0].linear", 0.09);
	lightingShader.SetFloat("pointLights[0].quadratic", 0.032);
	// point light 2
	lightingShader.SetVec3("pointLights[1].position", pointLightPositions[1]);
	lightingShader.SetVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.SetVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.SetVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.SetFloat("pointLights[1].constant", 1.0f);
	lightingShader.SetFloat("pointLights[1].linear", 0.09);
	lightingShader.SetFloat("pointLights[1].quadratic", 0.032);
	// point light 3
	lightingShader.SetVec3("pointLights[2].position", pointLightPositions[2]);
	lightingShader.SetVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.SetVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.SetVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.SetFloat("pointLights[2].constant", 1.0f);
	lightingShader.SetFloat("pointLights[2].linear", 0.09);
	lightingShader.SetFloat("pointLights[2].quadratic", 0.032);
	// point light 4
	lightingShader.SetVec3("pointLights[3].position", pointLightPositions[3]);
	lightingShader.SetVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.SetVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.SetVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.SetFloat("pointLights[3].constant", 1.0f);
	lightingShader.SetFloat("pointLights[3].linear", 0.09);
	lightingShader.SetFloat("pointLights[3].quadratic", 0.032);
	// spotLight
	lightingShader.SetVec3("spotLight.position", m_sceneCameraComp->GetCamera().GetPosition());
	lightingShader.SetVec3("spotLight.direction", m_sceneCameraComp->GetCamera().m_direction);
	lightingShader.SetVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	lightingShader.SetVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	lightingShader.SetVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	lightingShader.SetFloat("spotLight.constant", 1.0f);
	lightingShader.SetFloat("spotLight.linear", 0.09);
	lightingShader.SetFloat("spotLight.quadratic", 0.032);
	lightingShader.SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	lightingShader.SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

	// view/projection transformations
	lightingShader.SetMat4("projection", projection);
	lightingShader.SetMat4("view", view);

	// render containers
	for (unsigned int i = 0; i < 10; i++)
	{
		float angle = 20.0f * i;

		cubeObject.GetTransform().SetPosition(cubePositions[i]);
		cubeObject.GetTransform().RotateLocal(glm::vec3(1.0f, 0.3f, 0.5f), angle);
		
		lightingShader.SetMat4("model", cubeObject.GetTransform().GetModelMat());

		cubeObject.GetModel().Draw(lightingShader);
	}

	// also draw the lamp object(s)
	lampShader.Use();
	lampShader.SetMat4("projection", projection);
	lampShader.SetMat4("view", view);

	// we now draw as many light bulbs as we have point lights.
	for (unsigned int i = 0; i < 4; i++)
	{
		cubeObject.GetTransform().SetPosition(pointLightPositions[i]);
		cubeObject.GetTransform().Scale(0.2f);

		lampShader.SetMat4("model", cubeObject.GetTransform().GetModelMat());
		cubeObject.GetModel().Draw(lampShader);
	}

	// - skybox
	// glDepthFunc(GL_LEQUAL);
	// skyboxShader.Use();
	// glm::mat4 model = glm::scale(model, glm::vec3(1000.0f));
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

void MultipleLightsState::Cleanup()
{

}
