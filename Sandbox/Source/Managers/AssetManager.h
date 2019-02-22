#pragma once

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>

#include <thread>
#include <mutex>
#include <future>

#include "Dependencies/std_image/stb_image.h"

#include "Systems/Rendering/Shader.h"

#include "Systems/Model.h"
#include "Systems/Rendering/Texture.h"
#include "Systems/Material.h"

#include "Assets/AssimpImporter.h"
#include "Assets/SimpleTextureAssetJob.h"

struct TextureAssetJob
{
	unsigned int materialIndex = 0;
	TextureType textureType = TextureType::None;
	std::vector<std::string> resourcePaths;
	bool useGammaCorrection = true;
	std::vector<TextureInfo> texDatas;
};

struct TextureAssetJobResult
{
	unsigned int materialindex = 0;
	TextureType textureType = TextureType::None;
	std::vector<TextureInfo> textureInfos;
};

class Game;

class AssetManager
{
public:
	AssetManager();
	~AssetManager();

	void Initialize();

	void LoaderThread( std::future<void> futureObj );
	void Update();

	// Models
	std::shared_ptr<Model> LoadModel(const std::string& path);

	// Materials
	Material LoadMaterial(const std::string& materialPath);
	void LoadTexture(Material& material);
	
	// Textures
	Texture LoadTexture(const std::string& path, bool useGammaCorrection = false, TextureType type = TextureType::None);
	void LoadTextureAsync(const std::string& path, bool gammaCorrection, unsigned int* outId);

	// Cubemaps
	unsigned int LoadCubemap(const std::string& cubemapName, const std::vector<std::string>& paths);
	unsigned int LoadHDRTexure(const std::string& path);

private:
	TextureInfo LoadTextureFromFile(const std::string& path, bool gammaCorrection = false);
	Texture GenerateTexture(TextureInfo info, TextureType type = TextureType::None);
	
	std::vector<TextureInfo> LoadTexturesFromAssetJob(TextureAssetJob& job);

	nlohmann::json ReadJsonFile(const std::string& path);
	void SaveJsonFile(const std::string& path, const nlohmann::json& data);


private:
	AssimpImporter m_assimpImporter;
	std::vector<TextureType> m_supportedTextureTypes;

	std::unordered_map<size_t, Texture> m_textureMap;

	std::unordered_map<size_t, std::shared_ptr<Model>> m_modelsMap;
	std::unordered_map<size_t, unsigned int> m_textureCubeMap;

	static std::string s_assetDirectoryPath;
	static std::string s_shaderDirectoryPath;

	unsigned int m_defaultTex;

	std::queue<SimpleTextureAssetJob> m_jobsSimpleTextureAsset;
	std::queue<SimpleTextureAssetJobResult> m_jobsSimpleTextureResults;

	std::queue<TextureAssetJob> m_jobsTextureAssets;
	std::queue<TextureAssetJobResult> m_jobsTextureAssetResults;

	std::vector<std::shared_ptr<Mesh>> m_meshCache;
	std::vector<std::shared_ptr<Material>> m_materialCache;

	int m_minThreads = 1;
	int m_maxThreads = 2;

	std::vector<std::thread> m_workerThreads;
	std::unordered_map<std::thread::id, std::string> m_threadNames;

	std::promise<void> threadExitSignal;
	std::future<void> futureObj;

	bool m_loadingThreadActive;
	std::thread m_thread;
	std::mutex m_mutex;
	std::mutex m_stbiMutex;
	std::mutex m_setupTexMutex;
	std::mutex m_workloadReadyMutex;
};

