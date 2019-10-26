#include "AssetManager.h"

#include <algorithm>
#include <string>
#include <chrono>

#define OPTIMON 1
#define MULTITHREAD 0

const std::string AssetManager::s_assetDirectoryPath = "Data/";
const std::string AssetManager::s_shaderDirectoryPath = "Source/Shaders/";

AssetManager::AssetManager()
	: m_loadingThreadActive(true)
{
	m_properties.m_gammaCorrection = true;
	m_properties.m_flipHDROnLoad = true;

	m_supportedTextureTypes = {
		TextureType::DiffuseMap,
		TextureType::NormalMap,
		TextureType::SpecularMap,
		TextureType::MetallicMap,
		TextureType::RoughnessMap,
		TextureType::HeightMap,
		TextureType::AOMap,
	};

#if MULTITHREAD
	for (int i = 0; i < m_maxThreads; ++i)
	{
		m_workerThreads.push_back(std::thread(&AssetManager::LoaderThread, this));
	}

	m_threadNames.emplace(std::this_thread::get_id(), "Main");

	for (int i = 0; i < m_maxThreads; ++i)
	{
		std::string name = "Thread " + std::to_string(i);
		m_threadNames.emplace(m_workerThreads[i].get_id(), name);
	}
#endif
}

AssetManager::~AssetManager()
{
#if MULTITHREAD
	m_loadingThreadActive = false;
	for (int i = 0; i < m_maxThreads; ++i)
	{
		m_workerThreads[i].join();
	}
#endif
}

void AssetManager::Initialize()
{
	m_defaultTex = LoadTexture("Data/Images/default.jpg").GetId();
}

void AssetManager::LoaderThread()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	while (m_loadingThreadActive)
	{
		bool hasTextureAssetJob = false;
		bool hasSimpleTextureJob = false;

		TextureAssetJob textureAssetJob;
		SimpleTextureAssetJob simpleJob;

		unsigned int workLoadSizeRemaining = 0;

		// scope the lock for reading, that is written from another thread.
		{
			std::scoped_lock<std::mutex> lock(m_mutex);
			if (!m_jobsTextureAssets.empty())
			{
				textureAssetJob = m_jobsTextureAssets.front();
				m_jobsTextureAssets.pop();
				workLoadSizeRemaining += static_cast<unsigned int>(m_jobsTextureAssets.size());
				hasTextureAssetJob = true;
			}
			else if (!m_jobsSimpleTextureAsset.empty())
			{
				simpleJob = m_jobsSimpleTextureAsset.front();
				m_jobsSimpleTextureAsset.pop();
				workLoadSizeRemaining += static_cast<unsigned int>(m_jobsSimpleTextureAsset.size());
				hasSimpleTextureJob = true;
			}
		}

		if (hasTextureAssetJob)
		{
			TextureAssetJobResult jobResult;
			jobResult.materialindex = textureAssetJob.materialIndex;
			jobResult.textureInfos = LoadTexturesFromAssetJob(textureAssetJob);
			jobResult.textureType = textureAssetJob.textureType;

			{
				std::scoped_lock<std::mutex> lock(m_workloadReadyMutex);
				m_jobsTextureAssetResults.emplace(jobResult);
			}
		}

		if (hasSimpleTextureJob)
		{
			SimpleTextureAssetJobResult jobResult;
			jobResult.id = simpleJob.id;

			TextureLoadData textureLoadData;
			m_textureManager.LoadTexture(simpleJob.path, textureLoadData);

			jobResult.textureLoadData = textureLoadData;

			if (jobResult.textureLoadData.HasData())
			{
				std::scoped_lock<std::mutex> lock(m_setupTexMutex);
				m_jobsSimpleTextureResults.push(jobResult);
			}
		}

		{
			std::scoped_lock<std::mutex> lock(m_outputMutex);
			std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Jobs Remaining: " << workLoadSizeRemaining << '\n';
		}

		const int msToWait = workLoadSizeRemaining > 0 ? 10 : 2000;
		std::this_thread::sleep_for(std::chrono::milliseconds(msToWait));
	}
}

void AssetManager::Update()
{
	if (!m_jobsSimpleTextureResults.empty())
	{
		SimpleTextureAssetJobResult jobResult = m_jobsSimpleTextureResults.front();
		m_jobsSimpleTextureResults.pop();

		TextureInfo textureInfo = m_textureManager.GenerateTexture(jobResult.textureLoadData, TextureType::DiffuseMap, m_properties.m_gammaCorrection);

		// this should update the pointer. must find a better way to return the value
		// maybe a better approach would be using a callback.
		*jobResult.id = textureInfo.GetId();
	}

	if (!m_jobsTextureAssetResults.empty())
	{
		TextureAssetJobResult jobResult = m_jobsTextureAssetResults.front();
		m_jobsTextureAssetResults.pop();

		if (jobResult.materialindex >= m_materialCache.size()) 
		{
			return;
		}

		for (const TextureLoadData& textureLoadData : jobResult.textureInfos)
		{
			TextureInfo texture = m_textureManager.GenerateTexture(textureLoadData, jobResult.textureType, m_properties.m_gammaCorrection);
			m_materialCache[jobResult.materialindex]->AddTexture(texture);
		}
	}
}

std::shared_ptr<Model> AssetManager::LoadModel(const std::string& path)
{
	std::string lowercasePath = path;
	std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), ::tolower);

	size_t pathHash = std::hash<std::string>{}(lowercasePath);

	std::shared_ptr<Model> model = m_assimpImporter.LoadModel(lowercasePath);
	if (!model)
	{
		return std::shared_ptr<Model>();
	}

	m_modelsMap.emplace(pathHash, model);

	std::cout << "[AssetManager] Model: " << lowercasePath << " loaded." << std::endl;

	const auto& meshes = model->GetMeshes();
	const unsigned int meshCount = static_cast<unsigned int>(meshes.size());
	for (auto mesh : meshes)
	{
		unsigned int meshIndex = static_cast<unsigned int>(m_meshCache.size());
		unsigned int materialIndex = static_cast<unsigned int>(m_materialCache.size());

		auto material = mesh->GetMaterial();
		for (auto textureType : m_supportedTextureTypes)
		{
			std::vector<std::string> texturePaths = material->GetTexturePaths(textureType);

#if MULTITHREAD
			TextureAssetJob job;
			job.materialIndex = materialIndex;
			job.useGammaCorrection = false;
			job.textureType = textureType;
			job.resourcePaths = texturePaths;

			std::scoped_lock<std::mutex> lock(m_mutex);
			m_jobsTextureAssets.emplace(job);
#else
			for (const std::string& path : texturePaths)
			{
				TextureLoadData textureLoadData;
				m_textureManager.LoadTexture(path, textureLoadData);
				if (textureLoadData.HasData())
				{
					continue;
				}

				TextureInfo textureInfo = m_textureManager.GenerateTexture(textureLoadData, textureType, m_properties.m_gammaCorrection);
				if (textureInfo.IsValid())
				{
					continue;
				}

				material->AddTexture(textureInfo);
			}
#endif
		}

		m_materialCache.push_back(material);
		m_meshCache.push_back(mesh);
	}

	return model;
}

void AssetManager::LoadTexture(Material& material)
{
	for (TextureType type : m_supportedTextureTypes)
	{
		auto texturePaths = material.GetTexturePaths(type);
		for (const std::string& path : texturePaths)
		{
			TextureLoadData textureLoadData;
			m_textureManager.LoadTexture(path, textureLoadData);
			if (!textureLoadData.HasData())
			{
				continue;
			}
			
			TextureInfo texInfo = m_textureManager.GenerateTexture(textureLoadData, type, m_properties.m_gammaCorrection);
			if (!texInfo.IsValid())
			{
				continue;
			}

			material.AddTexture(texInfo);
		}
	}
}

TextureInfo AssetManager::LoadTexture(const std::string& path, TextureType type)
{
	std::string lowercasePath = path;
	GetLowercase(lowercasePath);

#if OPTIMON
	const TextureInfo texture = m_textureManager.FindTexture(lowercasePath);
	if (texture.IsValid())
	{
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture found" << std::endl;
		return texture;
	}
#endif

	TextureLoadData textureData;
	m_textureManager.LoadTexture(lowercasePath, textureData);
	if (!textureData.HasData())
	{
		std::cout << "[AssetManager][Error][" << m_threadNames[std::this_thread::get_id()] << "] Texture: " << path << " failed to load." << std::endl;
		return TextureInfo();
	}

	std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture: " << path << " loaded." << std::endl;
	
	return m_textureManager.GenerateTexture(textureData, type, m_properties.m_gammaCorrection);;
}


void AssetManager::LoadTextureAsync(const std::string& path, unsigned int* outId)
{
	std::string lowercasePath = path;
	GetLowercase(lowercasePath);

#if OPTIMON
	const TextureInfo texture = m_textureManager.FindTexture(lowercasePath);
	if (texture.IsValid())
	{
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture found" << std::endl;
		*outId = texture.GetId();
		return;
	}
#endif

	std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Creating Simple Texture Asset Job" << std::endl;

	SimpleTextureAssetJob job;
	job.path = lowercasePath;
	job.id = outId;

	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		m_jobsSimpleTextureAsset.push(job);
	}

	// return default texture while we load the intended one.
	// note: expand this to be handled by material, so we return
	// a temporary material instead. ( which in turn, has the temp tex )
	*outId = m_defaultTex;
}

unsigned int AssetManager::LoadCubemap(const std::string& cubemapName, const std::vector<std::string>& paths)
{
	unsigned int cubemapId = 0;
	const size_t nameHash = std::hash<std::string>{}(cubemapName);

	const unsigned int imageListSize = static_cast<unsigned int>(paths.size());
	std::vector<TextureLoadData> dataTextures;
	dataTextures.resize(imageListSize);

	bool success = true;
	for (unsigned int i = 0; i < imageListSize; ++i)
	{
		m_textureManager.LoadTexture(paths[i], dataTextures[i]);
		if (!dataTextures[i].HasData())
		{
			success = false;
			break;
		}
	}

	if (!success)
	{
		std::cout << "[AssetManager] One of the textures could not be loaded.\n";
		return cubemapId;
	}

	glGenTextures(1, &cubemapId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

	for (unsigned int i = 0; i < imageListSize; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
			dataTextures[i].m_width, 
			dataTextures[i].m_height, 
			0, GL_RGB, GL_UNSIGNED_BYTE, 
			dataTextures[i].m_dataPtr);

		stbi_image_free(dataTextures[i].m_dataPtr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	m_textureCubeMap.emplace(nameHash, cubemapId);
	return cubemapId;
}

unsigned int AssetManager::GetHDRTexture(const std::string& path)
{
	std::string lowercasePath = path;
	GetLowercase(lowercasePath);

#if OPTIMON
	const TextureInfo texture = m_textureManager.FindTexture(lowercasePath);
	if (texture.IsValid())
	{
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture found" << std::endl;
		return texture.GetId();
	}
#endif

	HDRTextureLoadData textureData;
	m_textureManager.LoadHDRTexture(lowercasePath, textureData);
	if (!textureData.HasData())
	{
		std::cout << "[AssetManager][Error][" << m_threadNames[std::this_thread::get_id()] << "] Texture: " << path << " failed to load." << std::endl;
		return 0;
	}

	return m_textureManager.GenerateHDRTexture(textureData, TextureType::DiffuseMap).GetId();
}

std::vector<TextureLoadData> AssetManager::LoadTexturesFromAssetJob(TextureAssetJob& job)
{
	std::vector<TextureLoadData> textureInfos;
	for (const auto& path : job.resourcePaths)
	{
		TextureLoadData textureInfo;
		m_textureManager.LoadTexture(path, textureInfo);
		if (textureInfo.HasData())
		{
			textureInfos.push_back(textureInfo);
		}
	}
	return textureInfos;
}

nlohmann::json AssetManager::ReadJsonFile(const std::string& path)
{
	std::stringstream stream;
	std::fstream file(path, std::fstream::in);
	if (file.is_open())
	{
		while (!file.eof())
		{
			std::string buffer;
			std::getline(file, buffer);
			stream << buffer;
		}
	}

	if (!stream.str().empty())
	{
		return json::parse(stream.str());
	}

	return json::object();
}

void AssetManager::SaveJsonFile(const std::string & path, const nlohmann::json& data)
{
	std::ofstream file;
	file.open(path, std::fstream::out);

	file << data.dump(4);
	file.close();
}

void AssetManager::GetLowercase(std::string& path)
{
	std::transform(path.begin(), path.end(), path.begin(), ::tolower);
}
