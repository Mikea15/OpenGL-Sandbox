
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Utils.h"

#include "Rendering/Vertex.h"

#include "Managers/AssetManager.h"

Model::Model()
	: m_useGammaCorrection(false)
{

}

Model::Model(const Model& copy)
	: m_useGammaCorrection(copy.m_useGammaCorrection)
	, m_meshes(copy.m_meshes)
{
}

Model::Model(Model&& move)
	: m_useGammaCorrection(move.m_useGammaCorrection)
	, m_meshes(move.m_meshes)
{
	
}

Model& Model::operator=(const Model& assign)
{
	m_useGammaCorrection = assign.m_useGammaCorrection;
	m_meshes = assign.m_meshes;
	return *this;
}

Mesh* Model::LoadMesh(const aiScene* rootScene, const aiMesh* mesh, Material* outMaterial)
{
	std::shared_ptr<Mesh> meshData = std::make_shared<Mesh>();

	std::vector<VertexInfo> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		VertexInfo vertex;
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		vertex.TexCoords = glm::vec2(0.0f);
		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoords = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	meshData->SetupMesh(vertices, indices);
	meshData->CreateBuffers();

	unsigned int newMeshIndex = m_meshes.size();

	auto iterator = m_meshToMaterial.emplace(newMeshIndex, std::make_shared<Material>());
	if (iterator.second)
	{
		outMaterial = iterator.first->second.get();
	}

	m_meshes.push_back(meshData);
	return m_meshes[newMeshIndex].get();
}

void Model::Draw(const Shader& shader)
{
	const int meshCount = m_meshes.size();
	for (unsigned int i = 0; i < meshCount; i++)
	{
		m_meshToMaterial[i]->SetShader(shader);
		m_meshes[i]->Draw(*m_meshToMaterial[i]);
	}
}

void Model::DrawInstanced(const Shader& shader, int instanceCount)
{
	const int meshCount = m_meshes.size();
	for (unsigned int i = 0; i < meshCount; i++)
	{
		m_meshes[i]->DrawInstanced(shader, instanceCount);
	}
}
