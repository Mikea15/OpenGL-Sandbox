#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "Systems/Rendering/Shader.h"

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	Shader LoadShader(const std::string& name, const std::string& vertex,
		const std::string& fragment, const std::string& geometry = "");
	Shader GetShader(const std::string& name);

private:
	std::unordered_map<std::string, Shader> m_shaderMap;

private:
	std::string s_shaderDirectory = "Data/Shaders/";
};