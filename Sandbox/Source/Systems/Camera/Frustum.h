#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

#include <vector>

enum class PlaneIntersectionType
{
	Front,
	Intersecting,
	Back
};

enum class ContainmentType
{
	Disjoint,
	Intersect,
	Contains
};

class Plane4
{
public:
	Plane4() {}
	Plane4(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	{
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;

		glm::vec3 cross = glm::cross(ab, ac);

		normal = glm::normalize(cross);
		d = -(glm::dot(normal, a));
	}

	Plane4(const glm::vec3& pointOnPlane, const glm::vec3& normal)
	{
		this->normal = normal;
		d = -(pointOnPlane.x * normal.x + pointOnPlane.y * normal.y + pointOnPlane.z * normal.z);
	}

	Plane4(float a, float b, float c, float d)
		: normal(glm::vec3(a, b, c)), d(d)
	{}

	~Plane4() {}

	void Normalize()
	{
		const float length = glm::length(normal);
		if (length > 0.0f)
		{
			const float factor = 1 / length;
			normal = normal * factor;
			d = d * factor;
		}
	}

	void Invert()
	{
		normal *= -1.0f;
		d *= -1.0f;
	}

	int Intersect(const glm::vec3& p)
	{
		float distance = ClassifyPoint(p);
		if (distance > 0) return 1;		// front side
		if (distance < 0) return -1;	// back side
		return 0;						// intersect
	}

	float ClassifyPoint(const glm::vec3& p)
	{
		return (p.x * normal.x) + (p.y * normal.y) + (p.z * normal.z) + d;
	}

	float PerpendicularDistance(const glm::vec3& p)
	{
		// dist = (ax + by + cz + d) / sqrt(a*a + b*b + c*c)
		return static_cast<float>(fabsf((normal.x * p.x + normal.y * p.y + normal.z * p.z) / sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z)));
	}

	glm::vec3 normal;
	float d;
};

class Frustum
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
	Frustum() {};
	// (view * projection)
	Frustum(const glm::mat4& viewProjection)
	{
		SetViewProjectionMatrix(viewProjection);
	}
	~Frustum() {};

	void SetViewProjectionMatrix(const glm::mat4& viewProjection)
	{
		viewProj = viewProjection;
		CreatePlanes();
		CreateCorners();
	}

	// plane needs to be normalized
	float DistanceToPlane(const glm::vec4& plane, const glm::vec3& point)
	{
		return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
	}

	// void SetCameraInternals(float angle, float ratio, float near, float far)
	// {
	// 	this->angle = angle;
	// 	this->ratio = ratio;
	// 	this->near = near;
	// 	this->far = far;
	// 
	// 	tang = static_cast<float>(tan(glm::radians(angle * 0.5f)));
	// 	nh = tang * near;
	// 	nw = nh * ratio;
	// 	fh = tang * far;
	// 	fw = fh * ratio;
	// }
	// 
	// void SetCameraDefinitions(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& right, const glm::vec3& up)
	// {
	// 	glm::vec3 f = glm::normalize(fwd);
	// 	glm::vec3 r = glm::normalize(right);
	// 	glm::vec3 u = glm::normalize(up);
	// 
	// 	glm::vec3 nc(0.0f), fc(0.0f);
	// 	nc = pos + f * near;
	// 	fc = pos + f * far;
	// 
	// 	ntl = nc + u * nh - r * nw;
	// 	ntr = nc + u * nh + r * nw;
	// 	nbl = nc - u * nh - r * nw;
	// 	nbr = nc - u * nh + r * nw;
	// 	ftl = fc + u * fh - r * fw;
	// 	ftr = fc + u * fh + r * fw;
	// 	fbl = fc - u * fh - r * fw;
	// 	fbr = fc - u * fh + r * fw;
	// 
	// 	m_planes[Top].Set3Points(ntr, ntl, ftl);
	// 	m_planes[Bottom].Set3Points(nbl, nbr, fbr);
	// 	m_planes[Left].Set3Points(ntl, nbl, fbl);
	// 	m_planes[Right].Set3Points(nbr, ntr, fbr);
	// 	m_planes[Near].Set3Points(ntl, ntr, nbr);
	// 	m_planes[Far].Set3Points(ftr, ftl, fbl);
	// }

	PlaneIntersectionType SphereIntersectPlane(const Plane4& plane, const glm::vec3& sphereOrigin, const float radius)
	{
		PlaneIntersectionType result = PlaneIntersectionType::Intersecting;
		float distance = glm::dot(plane.normal, sphereOrigin) + plane.d;
		if (distance > radius)
		{
			result = PlaneIntersectionType::Front;
		}
		else if (distance < -radius)
		{
			result = PlaneIntersectionType::Back;
		}
		return result;
	}

	ContainmentType SphereInFrustrum(const glm::vec3& origin, const float r)
	{
		int intersects = false;
		ContainmentType result;
		for (unsigned int i = 0; i < 6; ++i)
		{
			PlaneIntersectionType planeIntersectionType = SphereIntersectPlane(planes[i], origin, r);
			switch (planeIntersectionType)
			{
			case PlaneIntersectionType::Front:
				return ContainmentType::Disjoint;
			case PlaneIntersectionType::Intersecting:
				intersects = true;
				break;
			}
		}
		return intersects ? ContainmentType::Intersect : ContainmentType::Contains;
	}

	ContainmentType PointInFrustrum(const glm::vec3& point)
	{
		for (unsigned int i = 0; i < 6; ++i)
		{
			if (planes[i].ClassifyPoint(point) > 0)
			{
				return ContainmentType::Disjoint;
			}
		}
		return ContainmentType::Contains;
	}

	//int AABBInFrustrum(const glm::vec3& min, const glm::vec3& max)
	//{
	//	int in = 0, out = 0;
	//	glm::vec3 aabb[8];
	//	aabb[0] = min;
	//	aabb[1] = glm::vec3(min.x, max.y, min.z);
	//	aabb[2] = glm::vec3(min.x, max.y, max.z);
	//	aabb[3] = glm::vec3(min.x, min.y, max.z);
	//	aabb[4] = glm::vec3(max.x, max.y, min.z);
	//	aabb[5] = glm::vec3(max.x, min.y, min.z);
	//	aabb[6] = glm::vec3(max.x, min.y, max.z);
	//	aabb[7] = max;

	//	for (unsigned int i = 0; i < 6; ++i)
	//	{
	//		in = out = 0;
	//		for (unsigned int v = 0; v < 8; ++v)
	//		{
	//			if (m_planes[i].Distance(aabb[v]) < 0)
	//			{
	//				++out;
	//			}
	//			else
	//			{
	//				++in;
	//			}
	//		}
	//		if (in == 0)
	//		{
	//			return static_cast<int>(Intersection::Outside);
	//		}
	//		if (out > 0)
	//		{
	//			return static_cast<int>(Intersection::Intersect);
	//		}
	//	}
	//	return static_cast<int>(Intersection::Inside);
	//}

	//int AABBInFrustrumFast(const glm::vec3& min, const glm::vec3& max)
	//{
	//	for (unsigned int i = 0; i < 6; ++i)
	//	{
	//		for (unsigned int v = 0; v < 8; ++v)
	//		{
	//			if (m_planes[i].Distance(max) < 0)
	//			{
	//				return static_cast<int>(Intersection::Outside);
	//			}
	//			else if(m_planes[i].Distance(min) < 0)
	//			{
	//				return static_cast<int>(Intersection::Intersect);
	//			}
	//		}
	//	}
	//	return static_cast<int>(Intersection::Inside);
	//}

	void CreateCorners()
	{
		corners.resize(8);

		corners[0] = IntersectionPoint(planes[0], planes[2], planes[4]);
		corners[1] = IntersectionPoint(planes[0], planes[3], planes[4]);
		corners[2] = IntersectionPoint(planes[0], planes[3], planes[5]);
		corners[3] = IntersectionPoint(planes[0], planes[2], planes[5]);
		corners[4] = IntersectionPoint(planes[1], planes[2], planes[4]);
		corners[5] = IntersectionPoint(planes[1], planes[3], planes[4]);
		corners[6] = IntersectionPoint(planes[1], planes[3], planes[5]);
		corners[7] = IntersectionPoint(planes[1], planes[2], planes[5]);
	}

	void CreatePlanes()
	{
		planes.resize(6);

		planes[0] = Plane4(-viewProj[0][2], -viewProj[1][2], -viewProj[2][2], -viewProj[3][2]);
		planes[1] = Plane4(viewProj[0][2] - viewProj[0][3], viewProj[1][2] - viewProj[1][3], viewProj[2][2] - viewProj[2][3], viewProj[3][2] - viewProj[3][3]);
		planes[2] = Plane4(-viewProj[0][3] - viewProj[0][0], -viewProj[1][3] - viewProj[1][0], -viewProj[2][3] - viewProj[2][0], -viewProj[3][3] - viewProj[3][0]);
		planes[3] = Plane4(viewProj[0][0] - viewProj[0][3], viewProj[1][0] - viewProj[1][3], viewProj[2][0] - viewProj[2][3], viewProj[3][0] - viewProj[3][3]);
		planes[4] = Plane4(viewProj[0][1] - viewProj[0][3], viewProj[1][1] - viewProj[1][3], viewProj[2][1] - viewProj[2][3], viewProj[3][1] - viewProj[3][3]);
		planes[5] = Plane4(-viewProj[0][3] - viewProj[0][1], -viewProj[1][3] - viewProj[1][1], -viewProj[2][3] - viewProj[2][1], -viewProj[3][3] - viewProj[3][1]);

		planes[0].Normalize();
		planes[1].Normalize();
		planes[2].Normalize();
		planes[3].Normalize();
		planes[4].Normalize();
		planes[5].Normalize();
	}

	glm::vec3 IntersectionPoint(const Plane4& a, const Plane4& b, const Plane4& c)
	{
		// Formula used
		//    d1 ( N2 * N3 ) + d2 ( N3 * N1 ) + d3 ( N1 * N2 )
		//P = ------------------------------------------------
		//    N1 . ( N2 * N3 )
		//
		// Note: N refers to the normal, d refers to the displacement. '.' means dot product. '*' means cross product

		glm::vec3 xbc = glm::cross(b.normal, c.normal);
		const float f = -glm::dot(a.normal, xbc);

		glm::vec3 v1 = xbc * a.d;

		glm::vec3 xca = glm::cross(c.normal, a.normal);
		glm::vec3 v2 = xca * b.d;

		glm::vec3 xab = glm::cross(a.normal, b.normal);
		glm::vec3 v3 = xca * c.d;

		return glm::vec3(
			(v1.x + v2.x + v3.x) / f,
			(v1.y + v2.y + v3.y) / f,
			(v1.z + v2.z + v3.z) / f
		);
	}


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

private:
	glm::mat4 projection;
	glm::mat4 viewProj;
	std::vector<glm::vec3> corners;
	std::vector<Plane4> planes;
};

