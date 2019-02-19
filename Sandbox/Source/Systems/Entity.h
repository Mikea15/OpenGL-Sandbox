#pragma once

#include "Model.h"
#include "Transform.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Entity
{
public:
	Entity();
	Entity(const Entity& copy);
	~Entity() = default;

	void LoadModel(const std::string& modelPath);

	void SetTransform(const Transform& newTransform) { m_transform = newTransform; }
	Transform& GetTransform() { return m_transform; }

	Model& GetModel()
	{
		return m_model;
	}

	void SetModel(Model& m) 
	{ 
		m_model = m; 
	}

	// friend void to_json(json& j, const Entity& e);
	// friend void from_json(const json& j, Entity& e);

private:
	unsigned int m_id;

	Transform m_transform;

	// TODO: make a model component
	Model m_model;

	static unsigned int ID;
};

//void to_json(json& j, const Entity& e)
//{
//	j = json{
//		"transform", e.m_transform
//	};
//}
//
//void from_json(const json& j, Entity& e)
//{
//	j.at("transform").get_to(e.m_transform);
//}