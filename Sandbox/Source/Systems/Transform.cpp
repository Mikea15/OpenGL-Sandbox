#include "Transform.h"

void to_json(json& j, const Transform& e)
{
	j = json{
		"pos", 123
	};

	//j = json{
	//	{
	//		"m_position",
	//		{ "x", e.GetPosition().x },
	//		{ "y", e.GetPosition().y },
	//		{ "z", e.GetPosition().z }
	//	},
	//	{
	//		"m_scale",
	//		{ "x", e.GetScale().x },
	//		{ "y", e.GetScale().y },
	//		{ "z", e.GetScale().z }
	//	}
	//	/*,{
	//		"m_orientation",
	//		{ "x", e.m_orientation.x },
	//		{ "y", e.m_orientation.y },
	//		{ "z", e.m_orientation.z },
	//		{ "w", e.m_orientation.w }
	//	}*/
	//};
}

void from_json(const json& j, Transform& e)
{
	int a = 0;
	a = j["pos"];

	//glm::vec3 pos;
	//glm::vec3 scale;
	//// glm::quat orientation;

	//j.at("m_position").at("x").get_to(pos.x);
	//j.at("m_position").at("x").get_to(pos.y);
	//j.at("m_position").at("x").get_to(pos.z);

	//j.at("m_scale").at("x").get_to(scale.x);
	//j.at("m_scale").at("x").get_to(scale.y);
	//j.at("m_scale").at("x").get_to(scale.z);

	////j.at("m_orientation").at("x").get_to(orientation.x);
	////j.at("m_orientation").at("x").get_to(orientation.y);
	////j.at("m_orientation").at("x").get_to(orientation.z);
	////j.at("m_orientation").at("w").get_to(orientation.w);

	//e.SetPosition(pos);
	//e.SetScale(scale);
	//// e.SetOrientation(orientation);
}


Transform::Transform()
	: m_position(0.0f)
	, m_scale(1.0f)
{
	m_orientation = glm::quat(1.0f, 0, 0, 0);
}

Transform::Transform(const Transform& copy)
	: m_position(copy.m_position)
	, m_scale(copy.m_scale)
	, m_orientation(copy.m_orientation)
{
}

void Transform::SetPosition(const glm::vec3& position)
{
	m_position = position;
}

void Transform::SetScale(const glm::vec3& scale)
{
	m_scale = scale;
}

void Transform::SetOrientation(const glm::quat& orientation)
{
	m_orientation = orientation;
}

void Transform::Translate(const glm::vec3& deltaPosition)
{
	m_position += deltaPosition;
}

void Transform::RotateLocal(const glm::vec3& axis, float degrees)
{
	glm::quat rot = glm::angleAxis(glm::radians(degrees), axis);
	m_orientation = m_orientation * rot;
	m_orientation = glm::normalize(m_orientation);
}

void Transform::Scale(float delta)
{
	m_scale *= delta;
}

void Transform::Scale(const glm::vec3& axis, float delta)
{
	m_scale += axis * delta;
}

glm::mat4 Transform::GetModelMat()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, m_position);
	model = model * glm::toMat4(m_orientation);
	model = glm::scale(model, m_scale);
	return model;
}

