#pragma once

#include <vector>
#include <variant>

#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

#include "nlohmann/json.hpp"

class AssetManager;

using namespace nlohmann;


class Material
{
	struct MaterialProperty
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

	

private:
	Shader m_shader;
	std::vector<Texture> m_textures;

	std::unordered_map<unsigned int, unsigned int> m_materialMapToVectorIndex;
	std::vector<MaterialProperty> m_materialProperties;
};

