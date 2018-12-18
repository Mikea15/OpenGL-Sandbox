#pragma once

#include <vector>

#include <glm/glm.hpp>

class BoundingBox
{
public:
	BoundingBox();
	BoundingBox(const glm::vec3& origin, float halfSize);
	BoundingBox(const glm::vec3& origin, const glm::vec3& halfSize);
	BoundingBox(const glm::vec3& origin, const glm::vec3& min, const glm::vec3& max);
	~BoundingBox();

	bool Intersect(const glm::vec3& point) const;
	bool Intersect(const BoundingBox& aabb) const;

	const glm::vec3& GetPosition() const { return m_origin; }
	const float GetSize() const { return m_halfSize; }
	const float GetExtent() const { return fabsf(m_max.x - m_min.x); }

private:
	float m_halfSize;
	glm::vec3 m_origin;
	glm::vec3 m_min;
	glm::vec3 m_max;

	std::vector<glm::vec3> m_points;
};

