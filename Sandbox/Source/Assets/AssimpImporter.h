#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>
#include <vector>

#include "Systems/Rendering/Texture.h"

class Mesh;
class Model;
class Material;

class AssimpImporter
{
public:
	AssimpImporter();
	
	std::shared_ptr<Model> LoadModel(const std::string& path);
	std::shared_ptr<Mesh> LoadMesh(const aiScene* rootScene, const aiMesh* mesh);
	std::shared_ptr<Material> LoadMaterial( const aiMaterial* material, const std::string& dir);

	TextureType GetTextureTypeFrom(aiTextureType type);
	std::vector<aiTextureType> GetSupportedTypes() const { return m_supportedTypes; }

private:
	void ProcessModelNode(const aiScene* scene, aiNode* node, std::shared_ptr<Model> model, const std::string& directory);

private:
	std::unordered_map<aiTextureType, TextureType> m_typeConversion;
	std::vector<aiTextureType> m_supportedTypes;
};