#include "SceneManager.h"

#include "Systems/Entity.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::AddEntity(std::shared_ptr<Entity> entity)
{
	m_sceneObjects.push_back(entity);
}


void SceneManager::Update(float deltaTime)
{
	for (auto entity : m_sceneObjects)
	{
		// entity->Update(deltaTime);
	}
}

void SceneManager::Draw()
{
	for (auto entity : m_sceneObjects)
	{
		entity->Draw();
	}
}
