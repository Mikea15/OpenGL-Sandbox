
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

void Model::Initialize()
{
	const int meshCount = m_meshes.size();
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		m_meshes[i]->CreateBuffers();
	}
}

void Model::Draw(const Shader& shader)
{
	const int meshCount = m_meshes.size();
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		m_meshes[i]->Draw(shader);
	}
}

void Model::DrawInstanced(const Shader& shader, int instanceCount)
{
	const int meshCount = m_meshes.size();
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		m_meshes[i]->DrawInstanced(shader, instanceCount);
	}
}

void Model::AddMesh(std::shared_ptr<Mesh> mesh)
{
	m_meshes.push_back(mesh);
}
