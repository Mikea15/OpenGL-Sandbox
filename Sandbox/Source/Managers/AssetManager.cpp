#include "AssetManager.h"

#include <algorithm>
#include <string>
#include <chrono>

#define OPTIMON 1

std::string AssetManager::s_assetDirectoryPath = "Data/";
std::string AssetManager::s_shaderDirectoryPath = "Source/Shaders/";

AssetManager::AssetManager()
	: m_loadingThreadActive(true)
{
	futureObj = threadExitSignal.get_future();

	for (int i = 0; i < m_maxThreads; ++i)
	{
		m_workerThreads.push_back(std::thread(&AssetManager::LoaderThread, this, std::move(futureObj)));
	}

	m_threadNames.emplace(std::this_thread::get_id(), "Main");

	for (int i = 0; i < m_maxThreads; ++i)
	{
		std::string name = "Thread " + std::to_string(i);
		m_threadNames.emplace(m_workerThreads[i].get_id(), name);
	}
}

AssetManager::~AssetManager()
{
	threadExitSignal.set_value();
	m_loadingThreadActive = false;
	for (int i = 0; i < m_maxThreads; ++i)
	{
		m_workerThreads[i].join();
	}
}

void AssetManager::Initialize()
{
	m_defaultTex = LoadTexture("Data/Images/default.jpg", false).id;
}

void AssetManager::LoaderThread(std::future<void> futureObj)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	while (m_loadingThreadActive)
	{
		bool hasTextureAssetJob = false;
		bool hasSimpleTextureJob = false;

		TextureAssetJob textureAssetJob;
		SimpleTextureAssetJob simpleJob;

		int workLoadSizeRemaining = 0;

		// scope the lock for reading, that is writen from another thread.
		{
			std::scoped_lock<std::mutex> lock(m_mutex);
			if (!m_jobsTextureAssets.empty())
			{
				textureAssetJob = m_jobsTextureAssets.front();
				m_jobsTextureAssets.pop();
				workLoadSizeRemaining += m_jobsTextureAssets.size();
				hasTextureAssetJob = true;
			}
			else if (!m_jobsSimpleTextureAsset.empty())
			{
				simpleJob = m_jobsSimpleTextureAsset.front();
				m_jobsSimpleTextureAsset.pop();
				workLoadSizeRemaining += m_jobsSimpleTextureAsset.size();
				hasSimpleTextureJob = true;
			}
		}

		if (hasTextureAssetJob)
		{
			TextureAssetJobResult jobResult;
			jobResult.meshCacheIndex = textureAssetJob.meshCacheIndex;
			jobResult.textureInfos = LoadTexturesFromAssetJob(textureAssetJob);
			jobResult.textureType = GetTextureTypeFrom(textureAssetJob.textureType);
			
			{
				std::scoped_lock<std::mutex> lock(m_workloadReadyMutex);
				m_jobsTextureAssetResults.emplace(jobResult);
			}
		}

		if (hasSimpleTextureJob)
		{
			SimpleTextureAssetJobResult jobResult;
			jobResult.id = simpleJob.id;
			jobResult.textureInfo = LoadTextureFromFile(simpleJob.path, simpleJob.useGammaCorrection);
			if (jobResult.textureInfo.data != nullptr)
			{
				std::scoped_lock<std::mutex> lock(m_setupTexMutex);
				m_jobsSimpleTextureResults.push(jobResult);
			}
		}

		const int msToWait = workLoadSizeRemaining > 0 ? 10 : 2000;
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Jobs Remaining: " << workLoadSizeRemaining << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(msToWait));
	}
}

void AssetManager::Update()
{
	if (!m_jobsSimpleTextureResults.empty())
	{
		SimpleTextureAssetJobResult jobResult = m_jobsSimpleTextureResults.front();
		m_jobsSimpleTextureResults.pop();

		Texture texture = GenerateTexture(jobResult.textureInfo);
		
		// this should update the pointer. must find a better way to return the value
		// maybe a better approach would be using a callback.
		*jobResult.id = texture.id;
	}

	if (!m_jobsTextureAssetResults.empty())
	{
		TextureAssetJobResult jobResult = m_jobsTextureAssetResults.front();
		m_jobsTextureAssetResults.pop();

		std::vector<Texture> outTextures;
		for (TextureInfo ti : jobResult.textureInfos)
		{
			Texture tex = GenerateTexture(ti, jobResult.textureType);
			outTextures.push_back(tex);
		}

		if (jobResult.meshCacheIndex < m_meshCache.size() - 1)
		{
			// these meshes are not the same. check m_VAO on Mesh.
			m_meshCache[jobResult.meshCacheIndex]->SetupTextures(outTextures);
		}
	}
}

void AssetManager::LoadModel(const std::string& path, Model& model)
{
	std::string lowercasePath = path;
	std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), ::tolower);

	size_t pathHash = std::hash<std::string>{}(lowercasePath);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(lowercasePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "[AssetManager] Model: " << path << " failed to load." << std::endl;
		std::cout << "[Error] Assimp: " << importer.GetErrorString() << std::endl;
		return;
	}

	std::string currentDirectory = lowercasePath.substr(0, lowercasePath.find_last_of('/') + 1);

	ProcessModelNode(scene, scene->mRootNode, model, currentDirectory);
	m_modelsMap.emplace(pathHash, model);

	std::cout << "[AssetManager] Model: " << lowercasePath << " loaded." << std::endl;
}

Texture AssetManager::LoadTexture(const std::string& path, bool useGammaCorrection, TextureType type)
{
	std::string lowercasePath = path;
	std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), ::tolower);
	size_t pathHash = std::hash<std::string>{}(lowercasePath);

#if OPTIMON
	const auto findIt = m_textureMap.find(pathHash);
	if (findIt != m_textureMap.end())
	{
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture found." << std::endl;
		return findIt->second;
	}
#endif

	TextureInfo info = LoadTextureFromFile(lowercasePath, useGammaCorrection);
	if (info.data != nullptr)
	{
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture: " << path << " loaded." << std::endl;

		Texture texture = GenerateTexture(info, type);
		if (texture.id != 0)
		{
			m_textureMap.emplace(pathHash, texture);
		}
		return texture;
	}
	else
	{
		std::cout << "[AssetManager][Error][" << m_threadNames[std::this_thread::get_id()] << "] Texture: " << path << " failed to load." << std::endl;
	}

	stbi_image_free(info.data);
	return Texture();
}

void AssetManager::LoadTextureAsync(const std::string& path, bool gammaCorrection, unsigned int* outId)
{
	std::string lowercasePath = path;
	std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), ::tolower);
	size_t nameHash = std::hash<std::string>{}(lowercasePath);

	const auto findIt = m_textureMap.find(nameHash);
	if (findIt != m_textureMap.end())
	{
		std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Texture found" << std::endl;
		*outId = findIt->second.id;
		return;
	}
	
	std::cout << "[AssetManager][" << m_threadNames[std::this_thread::get_id()] << "] Creating Simple Texture Asset Job" << std::endl;

	SimpleTextureAssetJob job;
	job.path = lowercasePath;
	job.useGammaCorrection = gammaCorrection;
	job.id = outId;

	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		m_jobsSimpleTextureAsset.push(job);
	}

	// return detault texture while we load the intended one.
	// note: expand this to be handled by material, so we return
	// a temporary material instead. ( which in turn, has the temp tex )
	*outId = m_defaultTex;
}

unsigned int AssetManager::LoadCubemap(const std::string& cubemapName, const std::vector<std::string>& paths)
{
	unsigned int cubemapId = 0;

	// std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	size_t nameHash = std::hash<std::string>{}(cubemapName);

	glGenTextures(1, &cubemapId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

	int width = 0, height = 0, nrChannels = 0;
	const int imageListSize = paths.size();
	for (unsigned int i = 0; i < imageListSize; ++i)
	{
		unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data != nullptr)
		{
			std::cout << "[AssetManager] Cubemap texture: " << paths[i] << " loaded." << std::endl;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "[AssetManager] Cubemap texture: " << paths[i] << " failed to load." << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	m_textureCubeMap.emplace(nameHash, cubemapId);
	return m_textureCubeMap[nameHash];
}

unsigned int AssetManager::LoadHDRTexure(const std::string& path)
{
	unsigned int textureId = 0;

	stbi_set_flip_vertically_on_load(true);
	int width = 0, height = 0, nrChannels = 0;
	float *data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "[AssetManager] Texture: " << path << " failed to load HDR." << std::endl;
	}

	return textureId;
}


TextureInfo AssetManager::LoadTextureFromFile(const std::string& path, bool useGammaCorrection)
{
	TextureInfo info;
	info.path = path;
	info.gammaCorrection = useGammaCorrection;
	info.data = stbi_load(path.c_str(), &info.width, &info.height, &info.channels, 0);
	return info;
}

// call on main thread
Texture AssetManager::GenerateTexture(TextureInfo info, TextureType type)
{
	Texture texture;
	texture.textureType = type;
	
	GLenum internalFormat = 0;
	GLenum dataFormat = 0;
	if (info.channels == 1)
	{
		internalFormat = dataFormat = GL_RED;
	}
	else if (info.channels == 3)
	{
		internalFormat = info.gammaCorrection ? GL_SRGB : GL_RGB;
		dataFormat = GL_RGB;
	}
	else if (info.channels == 4)
	{
		internalFormat = info.gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
		dataFormat = GL_RGBA;
	}

	glGenTextures(1, &texture.id);

	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, info.width, info.height, 0, dataFormat, GL_UNSIGNED_BYTE, info.data);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(info.data);

	return texture;
}

std::vector<TextureInfo> AssetManager::LoadTexturesFromAssetJob(TextureAssetJob& job)
{
	std::vector<TextureInfo> textureInfos;
	for (const auto& path : job.resourcePaths)
	{
		TextureInfo textureInfo = LoadTextureFromFile(path, job.useGammaCorrection);
		if (textureInfo.data != nullptr) 
		{
			textureInfos.push_back(textureInfo);
		}
	}
	return textureInfos;
}

void AssetManager::ProcessModelNode(const aiScene* scene, aiNode* node, Model& model, const std::string& directory)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		Mesh* newMesh = model.LoadMesh(scene, mesh);

		m_meshCache.push_back(newMesh);
		const int meshCacheIndex = m_meshCache.size() - 1;

		auto diffuseTexJobs = TextureAssetJob();
		diffuseTexJobs.meshCacheIndex = meshCacheIndex;
		diffuseTexJobs.useGammaCorrection = model.m_useGammaCorrection;
		diffuseTexJobs.textureType = aiTextureType_DIFFUSE;
		LoadTexturesOfType(material, diffuseTexJobs.textureType, directory, diffuseTexJobs.resourcePaths);

		auto normalMapTexJobs = TextureAssetJob();
		normalMapTexJobs.meshCacheIndex = meshCacheIndex;
		normalMapTexJobs.useGammaCorrection = model.m_useGammaCorrection;
		normalMapTexJobs.textureType = aiTextureType_NORMALS;
		LoadTexturesOfType(material, normalMapTexJobs.textureType, directory, normalMapTexJobs.resourcePaths);

		auto specMapTexJobs = TextureAssetJob();
		specMapTexJobs.meshCacheIndex = meshCacheIndex;
		specMapTexJobs.useGammaCorrection = model.m_useGammaCorrection;
		specMapTexJobs.textureType = aiTextureType_SPECULAR;
		LoadTexturesOfType(material, specMapTexJobs.textureType, directory, specMapTexJobs.resourcePaths);

		auto heightMapTexJobs = TextureAssetJob();
		heightMapTexJobs.meshCacheIndex = meshCacheIndex;
		heightMapTexJobs.useGammaCorrection = model.m_useGammaCorrection;
		heightMapTexJobs.textureType = aiTextureType_HEIGHT;
		LoadTexturesOfType(material, heightMapTexJobs.textureType, directory, heightMapTexJobs.resourcePaths);

		bool useGammaCorrection = model.m_useGammaCorrection;
		// Synchronous Texture Loading
		bool enableAsyncLoading = false;
		if (enableAsyncLoading)
		{
			{
				std::scoped_lock<std::mutex> lock(m_mutex);
				if (!diffuseTexJobs.resourcePaths.empty())
				{
					m_jobsTextureAssets.emplace(diffuseTexJobs);
				}

				if (!normalMapTexJobs.resourcePaths.empty())
				{
					m_jobsTextureAssets.emplace(normalMapTexJobs);
				}

				if (!specMapTexJobs.resourcePaths.empty())
				{
					m_jobsTextureAssets.emplace(specMapTexJobs);
				}

				if (!heightMapTexJobs.resourcePaths.empty())
				{
					m_jobsTextureAssets.emplace(heightMapTexJobs);
				}
			}
		}
		else
		{
			std::vector<Texture> textures;
			for (const auto& p : diffuseTexJobs.resourcePaths)
			{
				Texture texture = LoadTexture(p, useGammaCorrection, TextureType::DiffuseMap);
				if (texture.id != 0)
				{
					textures.push_back(texture);
				}
			}

			for (const auto& p : specMapTexJobs.resourcePaths)
			{
				Texture texture = LoadTexture(p, useGammaCorrection, TextureType::SpecularMap);
				if (texture.id != 0)
				{
					textures.push_back(texture);
				}
			}

			for (const auto& p : heightMapTexJobs.resourcePaths)
			{
				Texture texture = LoadTexture(p, useGammaCorrection, TextureType::HeightMap);
				if (texture.id != 0)
				{
					textures.push_back(texture);
				}
			}

			for (const auto& p : normalMapTexJobs.resourcePaths)
			{
				Texture texture = LoadTexture(p, useGammaCorrection, TextureType::NormalMap);
				if (texture.id != 0)
				{
					textures.push_back(texture);
				}
			}
			newMesh->SetupTextures(textures);
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessModelNode(scene, node->mChildren[i], model, directory);
	}
}

void AssetManager::LoadTexturesOfType(const aiMaterial * material, aiTextureType textureType, std::string directory, std::vector<std::string>& texturePaths)
{
	const unsigned int textureCount = material->GetTextureCount(textureType);
	for (unsigned int i = 0; i < textureCount; ++i)
	{
		aiString path;
		material->GetTexture(textureType, i, &path);
		texturePaths.push_back(directory + path.C_Str());
	}
}




