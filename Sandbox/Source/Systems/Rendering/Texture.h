#pragma once

#include <string>
#include <unordered_map>

enum class TextureType
{
	None,
	DiffuseMap,
	NormalMap,
	SpecularMap,
	HeightMap,
	MetallicMap,
	AOMap,
	RoughnessMap
};

struct TextureInfo
{
	std::string path;
	bool gammaCorrection = true;

	int width = 0;
	int height = 0;
	int channels = 0;

	unsigned char* data = nullptr;
};

struct Texture
{
	unsigned int id = 0;
	TextureType textureType = TextureType::None;
};


