#pragma once

#include <random>

#include "DefaultState.h"

#include "Systems/QuadTree.h"

class IBLSpecState
	: public DefaultState
{
public:
	IBLSpecState();
	~IBLSpecState() override;

	void Init(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void Update(float deltaTime) override;
	void Render(float alpha = 1.0f) override;
	void RenderUI() override;
	void Cleanup() override;

private:
	QuadTree m_qTree;

	Shader pbrShader;
	Shader equirectangularToCubemapShader;
	Shader irradianceShader;
	Shader prefilterShader;
	Shader brdfShader;
	Shader backgroundShader;

	Shader wireframeShader;

	// load PBR material textures
	// --------------------------
	// rusted iron
	unsigned int ironAlbedoMap = 0;
	unsigned int ironNormalMap = 0;
	unsigned int ironMetallicMap = 0;
	unsigned int ironRoughnessMap = 0;
	unsigned int ironAOMap = 0;

	// gold
	unsigned int goldAlbedoMap = 0;
	unsigned int goldNormalMap = 0;
	unsigned int goldMetallicMap = 0;
	unsigned int goldRoughnessMap = 0;
	unsigned int goldAOMap = 0;

	// grass
	unsigned int grassAlbedoMap = 0;
	unsigned int grassNormalMap = 0;
	unsigned int grassMetallicMap = 0;
	unsigned int grassRoughnessMap = 0;
	unsigned int grassAOMap = 0;

	// plastic
	unsigned int plasticAlbedoMap = 0;
	unsigned int plasticNormalMap = 0;
	unsigned int plasticMetallicMap = 0;
	unsigned int plasticRoughnessMap = 0;
	unsigned int plasticAOMap = 0;

	// wall
	unsigned int wallAlbedoMap = 0;
	unsigned int wallNormalMap = 0;
	unsigned int wallMetallicMap = 0;
	unsigned int wallRoughnessMap = 0;
	unsigned int wallAOMap = 0;

	// marble
	unsigned int marbleAlbedoMap = 0;
	unsigned int marbleNormalMap = 0;
	unsigned int marbleHeightMap = 0;
	unsigned int marbleRoughnessMap;
	unsigned int marbleAOMap;

	// cliff granite
	unsigned int graniteAlbedoMap = 0;
	unsigned int graniteNormalMap = 0;
	unsigned int graniteHeightMap = 0;
	unsigned int graniteRoughnessMap = 0;
	unsigned int graniteAOMap = 0;

	// leather
	unsigned int leatherAlbedoMap = 0;
	unsigned int leatherNormalMap = 0;
	unsigned int leatherHeightMap = 0;
	unsigned int leatherRoughnessMap = 0;
	unsigned int leatherAOMap = 0;


	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;

	unsigned int captureFBO;
	unsigned int captureRBO;

	unsigned int hdrTexture;
	unsigned int envCubemap;

	Cube cube;

	glm::mat4 captureProjection;
	std::vector<glm::mat4> captureViews;
	Transform rotatingT;

	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUTTexture;

	int gX, gY, gZ;
	int gridSize = 50;
	float gridSpacing = 5.0f;
	std::vector<glm::vec3> positions;
	std::vector<ContainmentType> visible;
	Transform scratchTransform;

	unsigned int hdrFBO;
	unsigned int colorBuffer;
	unsigned int rboDepth;
	Camera topDownCamera;
};

