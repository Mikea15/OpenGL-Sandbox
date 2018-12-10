#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Rendering/Mesh.h"

class AssetManager;
class Shader;

class Model
{
public:
	Model();
	Model(const Model& copy);
	Model(Model&& move);
	Model& operator =(const Model& assign); 
	Model(std::string path) {};

	Mesh* LoadMesh(const aiScene* rootScene, const aiMesh* mesh);

	void Draw(const Shader& shader);
	void DrawInstanced(const Shader& shader, int instanceCount);

	std::vector<std::shared_ptr<Mesh>>& GetMeshes() { return m_meshes; }

	bool m_useGammaCorrection;
private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;

};
