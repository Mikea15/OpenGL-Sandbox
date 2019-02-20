#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>
#include <vector>

#include "Systems/Rendering/Texture.h"

class Mesh;
class Material;

class AssimpLoader
{
public:
	AssimpLoader();
	
	std::shared_ptr<Mesh> LoadMesh(const aiScene* rootScene, const aiMesh* mesh);
	std::shared_ptr<Material> LoadMaterial( const aiMaterial* material, const std::string& dir);

	TextureType GetTextureTypeFrom(aiTextureType type);
	std::vector<aiTextureType> GetSupportedTypes() { return m_supportedTypes; }

	std::unordered_map<aiTextureType, TextureType> m_typeConversion;
	std::vector<aiTextureType> m_supportedTypes;
};