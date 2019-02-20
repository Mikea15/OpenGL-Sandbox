#include "Mesh.h"

#include <fstream>
#include <sstream>

#include "Shader.h"

#include "Systems/Rendering/Vertex.h"

unsigned int Mesh::ID = 0;

Mesh::Mesh()
	: m_id(++ID)
	, m_isReady(false)
{
}

Mesh::Mesh(const Mesh& copy)
	: m_id(copy.m_id)
	, m_vertices(copy.m_vertices)
	, m_indices(copy.m_indices)
	, m_texture(copy.m_texture)
	, m_VAO(copy.m_VAO)
	, m_VBO(copy.m_VBO)
	, m_EBO(copy.m_EBO)
	, m_isReady(copy.m_isReady)
{
}

Mesh::Mesh(Mesh&& move)
	: m_id(move.m_id)
	, m_vertices(move.m_vertices)
	, m_indices(move.m_indices)
	, m_texture(move.m_texture)
	, m_VAO(move.m_VAO)
	, m_VBO(move.m_VBO)
	, m_EBO(move.m_EBO)
	, m_isReady(move.m_isReady)
{
}

Mesh& Mesh::operator=(const Mesh & assign)
{
	m_id = assign.m_id;
	m_vertices = assign.m_vertices;
	m_indices = assign.m_indices;
	m_texture = assign.m_texture;
	m_VAO = assign.m_VAO;
	m_VBO = assign.m_VBO;
	m_EBO = assign.m_EBO;
	m_isReady = assign.m_isReady;
	return *this;
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}

void Mesh::SetupMesh(std::vector<VertexInfo> vertices, std::vector<unsigned int> indices)
{
	this->m_vertices = vertices;
	this->m_indices = indices;
}

void Mesh::SetupTextures(std::vector<Texture> textures)
{
	this->m_texture = textures;
}

void Mesh::CreateBuffers()
{
	// create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(VertexInfo), &m_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

	unsigned int offset = 0;
	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(offset + 0); // layout (location = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(offset + 1); // layout (location = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(offset + 2); // layout (location = 2)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(offset + 3); // layout (location = 3)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(offset + 4); // layout (location = 4)
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(VertexInfo, Bitangent));

	glBindVertexArray(0);

	m_isReady = m_VAO != 0;
}


void Mesh::BindTextures(Shader shader)
{
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	for (unsigned int i = 0; i < m_texture.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name;
		switch (m_texture[i].textureType)
		{
		case TextureType::DiffuseMap:
			number = std::to_string(diffuseNr++);
			name = "texture_diffuse";
			break;
		case TextureType::HeightMap:
			number = std::to_string(heightNr++);
			name = "texture_height";
			break;
		case TextureType::NormalMap:
			number = std::to_string(normalNr++);
			name = "texture_normal";
			break;
		case TextureType::SpecularMap:
			number = std::to_string(specularNr++);
			name = "texture_specular";
			break;
		default: break;
		}

		std::string propertyName = (name + number);
		// now set the sampler to the correct texture unit
		shader.SetInt(propertyName, i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, m_texture[i].id);
	}
}

void Mesh::Draw(Material& material)
{
	if (!m_isReady) 
	{
		return;
	}

	material.BindTextures();

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::DrawInstanced(Material& material, int instanceCount)
{
	if (!m_isReady) 
	{
		return;
	}

	material.BindTextures();

	glBindVertexArray(m_VAO);
	glDrawElementsInstanced(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::Draw(Shader shader)
{
	if (!m_isReady) 
	{
		return;
	}

	BindTextures(shader);

	// draw mesh
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::DrawInstanced(Shader shader, int instanceCount)
{
	if (!m_isReady)
	{
		return;
	}

	BindTextures(shader);

	// draw mesh
	glBindVertexArray(m_VAO);
	glDrawElementsInstanced(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}


