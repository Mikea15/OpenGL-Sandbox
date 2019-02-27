#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Entity;

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void AddEntity(std::shared_ptr<Entity> entity);

	const std::vector<std::shared_ptr<Entity>>& GetSceneObjects() { return m_sceneObjects; }

	void Update(float deltaTime);
	void Draw(const glm::mat4& view, const glm::mat4& projection);
	void RenderUI();

private:
	std::vector<std::shared_ptr<Entity>> m_sceneObjects;
};