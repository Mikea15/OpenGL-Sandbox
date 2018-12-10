#include "Entity.h"

unsigned int Entity::ID = 0;

Entity::Entity()
	: m_id(++ID)
{
}

Entity::Entity(const Entity & copy)
{
	m_transform = copy.m_transform;
	m_model = copy.m_model;
}

void Entity::LoadModel(const std::string & modelPath)
{
	// m_model.LoadModel(modelPath);
}
