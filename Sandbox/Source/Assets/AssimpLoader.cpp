#include "AssimpLoader.h"
#include "Systems/Rendering/Mesh.h"

AssimpLoader::AssimpLoader()
{
	m_supportedTypes = {
		aiTextureType_DIFFUSE,
		aiTextureType_NORMALS,
		aiTextureType_SPECULAR,
		aiTextureType_HEIGHT
	};

	m_typeConversion =
	{
		{ aiTextureType_DIFFUSE, TextureType::DiffuseMap },
		{ aiTextureType_NORMALS, TextureType::NormalMap },
		{ aiTextureType_SPECULAR, TextureType::SpecularMap },
		{ aiTextureType_HEIGHT, TextureType::HeightMap }
	};
}

std::shared_ptr<Mesh> AssimpLoader::LoadMesh(const aiScene* rootScene, const aiMesh* mesh)
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

	return meshData;
}

std::shared_ptr<Material> AssimpLoader::LoadMaterial(const aiMaterial* material, const std::string& dir)
{
	std::shared_ptr<Material> newMaterial = std::make_shared<Material>();

	for (aiTextureType type : m_supportedTypes)
	{
		const unsigned int textureCount = material->GetTextureCount(type);
		for (unsigned int i = 0; i < textureCount; ++i)
		{
			aiString path;
			material->GetTexture(type, i, &path);
			newMaterial->AddTexturePath(m_typeConversion[type], dir + path.C_Str());
		}
	}

	return newMaterial;
}

TextureType AssimpLoader::GetTextureTypeFrom(aiTextureType type)
{
	return m_typeConversion[type];
}
