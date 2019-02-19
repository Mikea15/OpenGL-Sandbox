#pragma once

#include <string>
#include "Systems/Rendering/Texture.h"

struct SimpleTextureAssetJob
{
	std::string path = "";
	bool useGammaCorrection = true;
	unsigned int* id = nullptr;
};

struct SimpleTextureAssetJobResult
{
	TextureInfo textureInfo;
	unsigned int* id = nullptr;
};