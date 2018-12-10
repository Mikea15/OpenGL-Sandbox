#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vertex.h"
#include "Texture.h"
#include "../Material.h"

class Shader;

class Mesh 
{
public:
	Mesh();
	Mesh(const Mesh& copy);
	Mesh(Mesh&& move);

	Mesh& operator=(const Mesh& assign);

	virtual ~Mesh();

	virtual void SetupMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	virtual void SetupTextures(std::vector<Texture> textures);
	virtual void CreateBuffers();

	virtual void BindTextures(Shader shader);

	virtual void Draw(Material& material);
	virtual void DrawInstanced(Material& material, int instanceCount);

	virtual void Draw(Shader shader);
	virtual void DrawInstanced(Shader shader, int instanceCount);

	unsigned int GetVAO() const { return m_VAO; }
	unsigned int GetVBO() const { return m_VBO; }
	unsigned int GetEBO() const { return m_EBO; }

	std::vector<Vertex>& GetVertices() { return m_vertices; }
	std::vector<unsigned int>& GetIndices() { return m_indices; }
	std::vector<Texture>& GetTextures() { return m_texture; }

	const unsigned int GetId() const { return m_id; }

protected:
	Material m_material;

	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture> m_texture;

	unsigned int m_VAO; // vertex array object
	unsigned int m_VBO; // vertex buffer object
	unsigned int m_EBO; // element buffer object

private:
	unsigned int m_id;
	bool m_isReady;


	static unsigned int ID;
};
