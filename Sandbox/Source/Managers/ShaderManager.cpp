#include "ShaderManager.h"


ShaderManager::ShaderManager()
{
	LoadShader("default", "unlit/simple.vs", "unlit/color.fs");
	LoadShader("lighting", "lighting.vs", "lighting.fs");
	LoadShader("skybox", "skybox/skybox.vs", "skybox/skybox.fs");
	LoadShader("cubemaps", "skybox/cubemaps.vs", "skybox/cubemaps.fs");
	LoadShader("refraction", "lit/refraction.vs", "lit/refraction.fs");
	LoadShader("hdr", "postfx/hdr.vs", "postfx/hdr.fs");
}

ShaderManager::~ShaderManager()
{
}

Shader ShaderManager::LoadShader(const std::string& name, const std::string& vertex,
	const std::string& fragment, const std::string& geometry)
{
	if (name.empty() || vertex.empty() || fragment.empty())
	{
		std::cerr << "[ShaderManager] Wanna load a shader maybe?" << std::endl;
		return Shader();
	}

	const auto& findIt = m_shaderMap.find(name);
	if (findIt != m_shaderMap.end())
	{
		std::cerr << "[ShaderManager] Loading shader " << name << " already exist." << std::endl;
		return Shader();
	}

	std::string vertexPath = s_shaderDirectory + vertex;
	std::string fragmentPath = s_shaderDirectory + fragment;

	std::string geometryPath = s_shaderDirectory + geometry;

	Shader shader = Shader(vertexPath.c_str(), fragmentPath.c_str(), !geometry.empty() ? geometryPath.c_str() : nullptr );

	auto result = m_shaderMap.emplace(name, shader);
	if (result.second)
	{
		return result.first->second;
	}

	return Shader();
}

Shader ShaderManager::GetShader(const std::string& name)
{
	const auto& findIt = m_shaderMap.find(name);
	if (findIt != m_shaderMap.end())
	{
		return findIt->second;
		
	}
	return Shader();
}