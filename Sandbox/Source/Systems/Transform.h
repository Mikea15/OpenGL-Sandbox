#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Core/Utils.h"

class Transform
{
public:
	Transform();
	Transform(const Transform& copy);

	void SetPosition(const glm::vec3& position);
	const glm::vec3& GetPosition() const { return m_position; }

	void SetScale(const glm::vec3& scale);
	const glm::vec3& GetScale() const { return m_scale; }

	void SetOrientation(const glm::quat& orientation);
	const glm::quat& GetOrientation() const { return m_orientation; }

	void Translate(const glm::vec3& deltaPosition);
	void RotateLocal(const glm::vec3& axis, float degrees);

	void Scale(float delta);
	void Scale(const glm::vec3& axis, float delta);

	glm::mat4 GetModelMat();

	// friend void to_json(json& j, const Transform& e);
	// friend void from_json(const json& j, Transform& e);

private:
	glm::vec3 m_position;
	glm::vec3 m_scale;
	glm::quat m_orientation;
};

//void to_json(json& j, const Transform& e)
//{
//	j = json{
//		{ 
//			"m_position",
//			{ "x", e.m_position.x },
//			{ "y", e.m_position.y },
//			{ "z", e.m_position.z }
//		},
//		{
//			"m_scale",
//			{ "x", e.m_scale.x },
//			{ "y", e.m_scale.y },
//			{ "z", e.m_scale.z }
//		},
//		{
//			"m_orientation",
//			{ "x", e.m_orientation.x },
//			{ "y", e.m_orientation.y },
//			{ "z", e.m_orientation.z },
//			{ "w", e.m_orientation.w }
//		}
//	};
//}
//
//void from_json(const json& j, Transform& e)
//{
//	glm::vec3 pos;
//	glm::vec3 scale;
//	glm::quat orientation;
//
//	j.at("m_position").at("x").get_to(pos.x);
//	j.at("m_position").at("x").get_to(pos.y);
//	j.at("m_position").at("x").get_to(pos.z);
//
//	j.at("m_scale").at("x").get_to(scale.x);
//	j.at("m_scale").at("x").get_to(scale.y);
//	j.at("m_scale").at("x").get_to(scale.z);
//
//	j.at("m_orientation").at("x").get_to(orientation.x);
//	j.at("m_orientation").at("x").get_to(orientation.y);
//	j.at("m_orientation").at("x").get_to(orientation.z);
//	j.at("m_orientation").at("w").get_to(orientation.w);
//
//	e.SetPosition(pos);
//	e.SetScale(scale);
//	e.SetOrientation(orientation);
//}
