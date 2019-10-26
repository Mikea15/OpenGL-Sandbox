#pragma once

#include <queue>
#include <algorithm>
#include <string>
#include <functional>

#include <gl/glew.h>

#include "Dependencies/std_image/stb_image.h"

#include "Jobs/TextureLoaderJob.h"

struct TextureLoadData
{
	bool HasData() const { return m_dataPtr != nullptr; }

	std::string m_path;

	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;

	unsigned char* m_dataPtr = nullptr;
};

struct HDRTextureLoadData
{
	bool HasData() const { return m_dataPtr != nullptr; }

	std::string m_path;

	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;

	float* m_dataPtr = nullptr;
};

struct TextureInfo
{
	bool IsValid() const { return m_id != s_InvalidId; }

	unsigned int GetId() const { return m_id; }
	TextureType GetType() const { return m_textureType; }

	unsigned int m_id = s_InvalidId;
	TextureType m_textureType = TextureType::DiffuseMap;

	static const int s_InvalidId = -1;
};

class TextureManager
{
public:
	TextureManager() = default;

	void LoadTexture(const std::string& path, TextureLoadData& outData)
	{
		int height = 0, width = 0, channels = 0;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

		if (data != nullptr) 
		{
			outData.m_path = path;
			outData.m_height = height;
			outData.m_width = width;
			outData.m_channels = channels;
			outData.m_dataPtr = data;
		}
	}

	void LoadHDRTexture(const std::string& path, HDRTextureLoadData& outData, bool flipVertically = false)
	{
		stbi_set_flip_vertically_on_load(flipVertically);

		int width = 0, height = 0, channels = 0;
		float *data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
		if (data != nullptr)
		{
			outData.m_path = path;
			outData.m_height = height;
			outData.m_width = width;
			outData.m_channels = channels;
			outData.m_dataPtr = data;
		}
	}

	TextureInfo GenerateTexture(TextureLoadData textureData, TextureType type, bool useGammaCorrection)
	{
		TextureInfo texInfo;
		texInfo.m_textureType = type;

		GLenum internalFormat = 0;
		GLenum dataFormat = 0;
		if (textureData.m_channels == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (textureData.m_channels == 3)
		{
			internalFormat = useGammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (textureData.m_channels == 4)
		{
			internalFormat = useGammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glGenTextures(1, &texInfo.m_id);

		glBindTexture(GL_TEXTURE_2D, texInfo.m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 
			textureData.m_width, 
			textureData.m_height, 
			0, dataFormat, GL_UNSIGNED_BYTE, 
			textureData.m_dataPtr);

		stbi_image_free(textureData.m_dataPtr);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (texInfo.IsValid())
		{
			const size_t pathHash = std::hash<std::string>{}(textureData.m_path);
			m_textureMap.emplace(pathHash, texInfo);
			return texInfo;
		}

		return TextureInfo();
	}

	TextureInfo GenerateHDRTexture(HDRTextureLoadData textureData, TextureType type)
	{
		TextureInfo texInfo;
		texInfo.m_textureType = type;

		glGenTextures(1, &texInfo.m_id);

		glBindTexture(GL_TEXTURE_2D, texInfo.m_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 
			textureData.m_width, 
			textureData.m_height, 
			0, GL_RGB, GL_FLOAT, 
			textureData.m_dataPtr
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (texInfo.IsValid())
		{
			const size_t pathHash = std::hash<std::string>{}(textureData.m_path);
			m_textureMap.emplace(pathHash, texInfo);
			return texInfo;
		}

		return TextureInfo();
	}

	TextureInfo FindTexture(const std::string& path) const
	{
		const size_t pathHash = std::hash<std::string>{}(path);
		const auto findIt = m_textureMap.find(pathHash);
		if (findIt != m_textureMap.end())
		{
			return findIt->second;
		}
		return TextureInfo();
	}
	
private:
	std::unordered_map<size_t, TextureInfo> m_textureMap;

	std::queue<TextureLoaderJob*> m_workQueue;
};