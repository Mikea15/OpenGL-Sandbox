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
	// std::copy(textures.begin(), textures.end(), m_textures.begin());
	//m_textures = textures;
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
		Property mp;
		mp.name = name;
		mp.value = value;
		unsigned int index = m_materialProperties.size();
		m_materialProperties.push_back(mp);
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

/*
	Support for only one texture per texture type
*/
void Material::BindTextures()
{
	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		switch (m_textures[i].textureType)
		{
		case TextureType::DiffuseMap:
			glActiveTexture(GL_TEXTURE3);
			m_shader.SetInt("albedoMap", 3);
			break;
		case TextureType::NormalMap:
			glActiveTexture(GL_TEXTURE4);
			m_shader.SetInt("normalMap", 4);
			break;
		case TextureType::MetallicMap:
			glActiveTexture(GL_TEXTURE5);
			m_shader.SetInt("metallicMap", 5);
			break;
		case TextureType::RoughnessMap:
			glActiveTexture(GL_TEXTURE6);
			m_shader.SetInt("roughnessMap", 6);
			break;
		case TextureType::AOMap:
			glActiveTexture(GL_TEXTURE7);
			m_shader.SetInt("aoMap", 7);
			break;
		case TextureType::HeightMap:
			glActiveTexture(GL_TEXTURE8);
			m_shader.SetInt("heightMap", 8);
			break;
		case TextureType::SpecularMap:
			glActiveTexture(GL_TEXTURE9);
			m_shader.SetInt("specularMap", 9);
			break;
		default: break;
		}

		glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
	}
}

void Material::AddTexture(Texture texture)
{
	m_textures.push_back(texture);
}

void Material::AddTexturePath(TextureType type, const std::string& path)
{
	m_texturePathPerType[type].push_back(path);
}






