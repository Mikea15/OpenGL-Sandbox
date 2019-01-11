#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

#include <vector>

#include "Plane.h"

class BoundingBox;

class BoundingFrustum
{
	enum
	{
		Top = 0,
		Bottom,
		Left,
		Right,
		Near,
		Far
	};

public:
	BoundingFrustum();

	// (view * projection)
	BoundingFrustum(const glm::mat4& viewProjection);

	~BoundingFrustum();

	void SetViewProjectionMatrix(const glm::mat4& viewProjection);

	bool Intersects(const BoundingBox& aabb) const;
	bool Intersects(const BoundingFrustum& frustum) const;
	PlaneIntersectionType Intersects(const Plane& plane) const;

	ContainmentType Contains(const glm::vec3& point) const;
	ContainmentType Contains(const BoundingBox& aabb) const;
	ContainmentType Contains(const BoundingFrustum& frustum) const;

	void CreateCorners();
	void CreatePlanes();

	glm::vec3 IntersectionPoint(const Plane& a, const Plane& b, const Plane& c);

	void DrawPoints();
	void DrawLines();
	void DrawPlanes();
	void DrawNormals();

	// Plane4 m_planes[6];
	// near top/bottom left/right
	glm::vec3 ntl, ntr, nbl, nbr;
	// far top/bottom left/right
	glm::vec3 ftl, ftr, fbl, fbr;
	// near/far distance, aspect ratio, fov angle, tangent something
	float near, far, ratio, angle, tang;
	// near/far width, near/far height
	float nw, nh, fw, fh;

	unsigned int vao = 0, vbo = 0, ebo = 0;

	const std::vector<glm::vec3> GetCorners() const { return corners; }

private:
	glm::mat4 projection;
	glm::mat4 viewProj;
	std::vector<glm::vec3> corners;
	std::vector<Plane> planes;
};

