#include "Material.h"

#include "Managers/AssetManager.h"

void to_json(json& j, const Material& p)
{
	j = json::object();

	std::vector<Material::IntermediateData> data;
	data.push_back({ TextureType::DiffuseMap, p.GetTexturePathsConst(TextureType::DiffuseMap) });
	data.push_back({ TextureType::NormalMap, p.GetTexturePathsConst(TextureType::NormalMap) });
	data.push_back({ TextureType::HeightMap, p.GetTexturePathsConst(TextureType::HeightMap) });
	data.push_back({ TextureType::MetallicMap, p.GetTexturePathsConst(TextureType::MetallicMap) });
	data.push_back({ TextureType::RoughnessMap, p.GetTexturePathsConst(TextureType::RoughnessMap) });
	data.push_back({ TextureType::SpecularMap, p.GetTexturePathsConst(TextureType::SpecularMap) });
	data.push_back({ TextureType::AOMap, p.GetTexturePathsConst(TextureType::AOMap) });

	j["Material"] = data;
}

void from_json(const json& j, Material& p)
{
	std::vector<Material::IntermediateData> data = j.at("Material").get<std::vector<Material::IntermediateData>>();

	for (auto element : data)
	{
		for (auto paths : element.data)
		{
			p.AddTexturePath(element.type, paths);
		}
	}
}

void to_json(json& j, const Material::IntermediateData& p)
{
	j = json::object();
	j["TextureType"] = p.type;
	j["TexturePaths"] = p.data;
}

void from_json(const json& j, Material::IntermediateData& p)
{
	p.type = j.at("TextureType").get<TextureType>();
	p.data = j.at("TexturePaths").get<std::vector<std::string>>();
}

Material::Material()
{
}

void Material::SetShader(const Shader& shader)
{
	m_shader = shader;
}

void Material::SetMVP(const glm::mat4& model, const glm::mat4& view, const glm::mat4 & projection)
{
	m_shader.Use();
	m_shader.SetMat4("view", view);
	m_shader.SetMat4("projection", projection);
	m_shader.SetMat4("model", model);
}

void Material::SetMaterialProperty(const std::string& name, const std::variant<int, float, std::string, bool>& value)
{
	unsigned int hash = std::hash<std::string>{}(name);

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
	m_shader.Use();
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

void Material::AddTexturePaths(TextureType type, const std::vector<std::string>& paths)
{
	m_texturePathPerType[type] = paths;
}






