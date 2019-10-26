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

#include "Assets/Jobs/Job.h"
#include "Assets/Jobs/TextureLoaderJob.h"
#include "Assets/TextureManager.h"

#include "Assets/AssimpImporter.h"
#include "Assets/SimpleTextureAssetJob.h"

struct TextureAssetJob
{
	unsigned int materialIndex = 0;
	TextureType textureType = TextureType::None;
	std::vector<std::string> resourcePaths;
	bool useGammaCorrection = true;
	std::vector<TextureLoadData> texDatas;
};

struct TextureAssetJobResult
{
	unsigned int materialindex = 0;
	TextureType textureType = TextureType::None;
	std::vector<TextureLoadData> textureInfos;
};

class Game;

class AssetManager
{
public:
	struct Properties
	{
		bool m_gammaCorrection;
		bool m_flipHDROnLoad;
	};

	AssetManager();
	~AssetManager();

	void Initialize();

	void LoaderThread( );

	void Update();

	// Models
	std::shared_ptr<Model> LoadModel(const std::string& path);

	// Materials
	Material LoadMaterial(const std::string& materialPath);
	void LoadTexture(Material& material);
	
	// Textures
	TextureInfo LoadTexture(const std::string& path, TextureType type = TextureType::DiffuseMap);
	void LoadTextureAsync(const std::string& path, unsigned int* outId);

	// Cubemaps
	unsigned int LoadCubemap(const std::string& cubemapName, const std::vector<std::string>& paths);
	
	unsigned int GetHDRTexture(const std::string& path);

private:
	std::vector<TextureLoadData> LoadTexturesFromAssetJob(TextureAssetJob& job);

	nlohmann::json ReadJsonFile(const std::string& path);
	void SaveJsonFile(const std::string& path, const nlohmann::json& data);

	void GetLowercase(std::string& path);

private:
	Properties m_properties;
	TextureManager m_textureManager;

	AssimpImporter m_assimpImporter;
	std::vector<TextureType> m_supportedTextureTypes;

	std::unordered_map<size_t, std::shared_ptr<Model>> m_modelsMap;
	std::unordered_map<size_t, unsigned int> m_textureCubeMap;

	unsigned int m_defaultTex;

	std::queue<std::shared_ptr<Job>> m_pendingJobs;
	std::mutex m_pendingJobsMutex;

	std::queue<SimpleTextureAssetJob> m_jobsSimpleTextureAsset;
	std::queue<SimpleTextureAssetJobResult> m_jobsSimpleTextureResults;

	std::queue<TextureAssetJob> m_jobsTextureAssets;
	std::queue<TextureAssetJobResult> m_jobsTextureAssetResults;

	std::vector<std::shared_ptr<Mesh>> m_meshCache;
	std::vector<std::shared_ptr<Material>> m_materialCache;

	int m_maxThreads = 6;

	std::vector<std::thread> m_workerThreads;
	std::unordered_map<std::thread::id, std::string> m_threadNames;

	bool m_loadingThreadActive;
	std::thread m_thread;
	std::mutex m_mutex;
	std::mutex m_stbiMutex;
	std::mutex m_setupTexMutex;
	std::mutex m_workloadReadyMutex;

	std::mutex m_outputMutex;

	// ---
	static const std::string s_assetDirectoryPath;
	static const std::string s_shaderDirectoryPath;
};

