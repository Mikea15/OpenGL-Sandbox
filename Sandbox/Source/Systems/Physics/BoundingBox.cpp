#include "BoundingBox.h"



BoundingBox::BoundingBox()
	: BoundingBox(glm::vec3(0.0f), 1.0f)
{
}

BoundingBox::BoundingBox(const glm::vec3& origin, float halfSize)
	: m_origin(origin), m_halfSize(halfSize)
{
	m_points.resize(8);
	m_points[0] = m_min =	m_origin + glm::vec3(-m_halfSize, -m_halfSize, -m_halfSize);
	m_points[3] =			m_origin + glm::vec3(-m_halfSize, -m_halfSize,  m_halfSize);
	m_points[1] =			m_origin + glm::vec3(-m_halfSize,  m_halfSize, -m_halfSize);
	m_points[2] =			m_origin + glm::vec3(-m_halfSize,  m_halfSize,  m_halfSize);
	m_points[5] =			m_origin + glm::vec3( m_halfSize, -m_halfSize, -m_halfSize);
	m_points[6] =			m_origin + glm::vec3( m_halfSize, -m_halfSize,  m_halfSize);
	m_points[4] =			m_origin + glm::vec3( m_halfSize,  m_halfSize, -m_halfSize);
	m_points[7] = m_max =	m_origin + glm::vec3( m_halfSize,  m_halfSize,  m_halfSize);
}

BoundingBox::BoundingBox(const glm::vec3& origin, const glm::vec3& halfSize)
{
	m_halfSize = glm::length(halfSize);
	m_points.resize(8);
	m_points[0] = m_min =	m_origin + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z);
	m_points[3] =			m_origin + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z);
	m_points[1] =			m_origin + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z);
	m_points[2] =			m_origin + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z);
	m_points[5] =			m_origin + glm::vec3( halfSize.x, -halfSize.y, -halfSize.z);
	m_points[6] =			m_origin + glm::vec3( halfSize.x, -halfSize.y,  halfSize.z);
	m_points[4] =			m_origin + glm::vec3( halfSize.x,  halfSize.y, -halfSize.z);
	m_points[7] = m_max =	m_origin + glm::vec3( halfSize.x,  halfSize.y,  halfSize.z);
}

BoundingBox::BoundingBox(const glm::vec3& origin, const glm::vec3& min, const glm::vec3& max)
{
	m_points.resize(8);
	m_points[0] = m_min = origin + min;
	m_points[3] = m_origin + glm::vec3(min.x, min.y, max.z);
	m_points[1] = m_origin + glm::vec3(min.x, max.y, min.z);
	m_points[2] = m_origin + glm::vec3(min.x, max.y, max.z);
	m_points[5] = m_origin + glm::vec3(max.x, min.y, min.z);
	m_points[6] = m_origin + glm::vec3(max.x, min.y, max.z);
	m_points[4] = m_origin + glm::vec3(max.x, max.y, min.z);
	m_points[7] = m_max = origin + max;

	m_halfSize = GetExtent() * 0.5f;
}

BoundingBox::~BoundingBox()
{
	m_points.clear();
}

bool BoundingBox::Intersect(const glm::vec3& point) const
{
	if (point.x < m_min.x || point.x > m_max.x) return false;
	if (point.y < m_min.y || point.y > m_max.y) return false;
	if (point.z < m_min.z || point.z > m_max.z) return false;

	return true;
}

bool BoundingBox::Intersect(const BoundingBox& aabb) const
{
	if (aabb.m_max.x < m_min.x || aabb.m_min.x > m_max.x) return false;
	if (aabb.m_max.y < m_min.y || aabb.m_min.y > m_max.y) return false;
	if (aabb.m_max.z < m_min.z || aabb.m_min.z > m_max.z) return false;

	return true;
}
