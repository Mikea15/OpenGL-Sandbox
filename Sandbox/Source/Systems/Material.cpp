#include "Material.h"

#include "Managers/AssetManager.h"

void to_json(json & j, const Material & mat)
{
	j = json{
		"MaterialProps", 1
	};
}

void from_json(const json & j, Material & mat)
{
	int var = 0;
	j.at("MaterialProps").get_to(var);
}



Material::Material()
{
}

void Material::SetTextures(const std::vector<Texture>& textures)
{
	m_textures = textures;
}

void Material::SetShader(const Shader& shader)
{
	m_shader = shader;
}

void Material::SetMaterialProperty(const std::string& name, const std::variant<int, float, std::string, bool>& value)
{
	unsigned int hash = std::hash < std::string>{}(name);

	auto findIt = m_materialMapToVectorIndex.find(hash);
	if (findIt == m_materialMapToVectorIndex.end())
	{
		MaterialProperty mp;
		mp.name = name;
		mp.value = value;
		m_materialProperties.push_back(mp);
		unsigned int index = m_materialProperties.size() - 1;
		m_materialMapToVectorIndex.emplace(hash, index);
	}
	else
	{
		int index = findIt->second;
		if (index < m_materialProperties.size())
		{
			m_materialProperties[index].value = value;
		}
	}
}

void Material::BindTextures()
{
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name;
		switch (m_textures[i].textureType)
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
		m_shader.SetInt(propertyName, i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
	}
}




