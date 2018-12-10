#pragma once

#include <iostream>
#include <vector>

class Entity;

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	const std::vector<Entity*>& GetSceneObjects() { return m_sceneObjects; }

private:
	std::vector<Entity*> m_sceneObjects;
};