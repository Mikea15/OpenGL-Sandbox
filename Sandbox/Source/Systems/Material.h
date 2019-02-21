#pragma once

#include <vector>
#include <variant>

#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

#include "nlohmann/json.hpp"

using namespace nlohmann;

class Material
{
	struct Property
	{
		std::string name;
		std::variant<int, float, std::string, bool> value;
	};

public:
	Material();
	~Material() = default;

	void SetShader(const Shader& shader);

	void SetTextures(const std::vector<Texture>& textures);
	void SetMaterialProperty(const std::string& name, const std::variant<int, float, std::string, bool>& value);

	void BindTextures();

	void AddTexture(Texture texture);
	void AddTexturePath(TextureType type, const std::string& path);

	std::vector<std::string> GetTexturePaths(TextureType type) { return m_texturePathPerType[type]; }

private:
	Shader m_shader;
	std::vector<Texture> m_textures;

	std::unordered_map<TextureType, std::vector<std::string>> m_texturePathPerType;

	std::unordered_map<unsigned int, unsigned int> m_materialMapToVectorIndex;
	std::vector<Property> m_materialProperties;
};

