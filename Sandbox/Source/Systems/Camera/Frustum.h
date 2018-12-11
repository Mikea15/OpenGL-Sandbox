#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

class Plane4
{
public:
	Plane4() {}
	Plane4(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) 
	{
		Set3Points(v1, v2, v3);
	}
	~Plane4() {}

	void Set3Points(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) 
	{
		glm::vec3 aux1, aux2;
		aux1 = v1 - v2;
		aux2 = v3 - v2;

		normal = glm::normalize(aux2 * aux1);


		point = v2;
		d = -glm::dot(normal, point);
	}

	void SetNormalAndPoint(const glm::vec3& normal, const glm::vec3& point) 
	{
		this->normal = glm::normalize(normal);
		this->point = point;
		d = -glm::dot(normal, point);
	}

	void SetCoefficients(float a, float b, float c, float d) 
	{
		normal = glm::vec3(a, b, c);
		float len = glm::length(normal);
		if (len > 0.0f) 
		{
			normal = normal / len;
			d = d / len;
		}
	}

	float Distance(const glm::vec3& p) 
	{
		return d + glm::dot(normal, p);
	}

	glm::vec3 normal, point;
	float d;
};

class Frustum
{
	enum class Intersection
	{
		Outside, 
		Intersect, 
		Inside
	};

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
	~Frustum() {};

	// plane needs to be normalized
	float DistanceToPlane(const glm::vec4& plane, const glm::vec3& point)
	{
		return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
	}

	void SetCameraInternals(float angle, float ratio, float near, float far)
	{
		this->angle = angle;
		this->ratio = ratio;
		this->near = near;
		this->far = far;

		tang = static_cast<float>(tan(glm::radians(angle * 0.5f)));
		nh = tang * near;
		nw = nh * ratio;
		fh = tang * far;
		fw = fh * ratio;
	}

	void SetCameraDefinitions(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& right, const glm::vec3& up)
	{
		glm::vec3 nc, fc;
		nc = pos + fwd * near;
		fc = pos + fwd * far;

		ntl = nc + up * nh - right * nw;
		ntr = nc + up * nh + right * nw;
		nbl = nc - up * nh - right * nw;
		nbr = nc - up * nh + right * nw;

		ftl = fc + up * fh - right * fw;
		ftr = fc + up * fh + right * fw;
		fbl = fc - up * fh - right * fw;
		fbr = fc - up * fh + right * fw;

		m_planes[Top].Set3Points(ntr, ntl, ftl);
		m_planes[Bottom].Set3Points(nbl, nbr, fbr);
		m_planes[Left].Set3Points(ntl, nbl, fbl);
		m_planes[Right].Set3Points(nbr, ntr, fbr);
		m_planes[Near].Set3Points(ntl, ntr, nbr);
		m_planes[Far].Set3Points(ftr, ftl, fbl);
	}

	int PointInFrustrum(const glm::vec3& point)
	{
		for (unsigned int i = 0; i < 6; ++i)
		{
			if (m_planes[i].Distance(point) < 0)
			{
				return static_cast<int>(Intersection::Outside);
			}
		}
		return static_cast<int>(Intersection::Inside);
	}

	int SphereInFrustrum(const glm::vec3& origin, const float r)
	{
		float distance = 0.0f;
		for (unsigned int i = 0; i < 6; ++i)
		{
			distance = m_planes[i].Distance(origin);
			if (distance < -r) 
			{
				return static_cast<int>(Intersection::Outside);
			}
			else if (distance < r) 
			{
				return static_cast<int>(Intersection::Intersect);
			}
		}
		return static_cast<int>(Intersection::Inside);
	}

	int AABBInFrustrum(const glm::vec3& min, const glm::vec3& max)
	{
		int in = 0, out = 0;
		glm::vec3 aabb[8];
		aabb[0] = min;
		aabb[1] = glm::vec3(min.x, max.y, min.z);
		aabb[2] = glm::vec3(min.x, max.y, max.z);
		aabb[3] = glm::vec3(min.x, min.y, max.z);
		aabb[4] = glm::vec3(max.x, max.y, min.z);
		aabb[5] = glm::vec3(max.x, min.y, min.z);
		aabb[6] = glm::vec3(max.x, min.y, max.z);
		aabb[7] = max;

		for (unsigned int i = 0; i < 6; ++i)
		{
			in = out = 0;
			for (unsigned int v = 0; v < 8; ++v)
			{
				if (m_planes[i].Distance(aabb[v]) < 0)
				{
					++out;
				}
				else
				{
					++in;
				}
			}
			if (in == 0)
			{
				return static_cast<int>(Intersection::Outside);
			}
			if (out > 0)
			{
				return static_cast<int>(Intersection::Intersect);
			}
		}
		return static_cast<int>(Intersection::Inside);
	}

	int AABBInFrustrumFast(const glm::vec3& min, const glm::vec3& max)
	{
		for (unsigned int i = 0; i < 6; ++i)
		{
			for (unsigned int v = 0; v < 8; ++v)
			{
				if (m_planes[i].Distance(max) < 0)
				{
					return static_cast<int>(Intersection::Outside);
				}
				else if(m_planes[i].Distance(min) < 0)
				{
					return static_cast<int>(Intersection::Intersect);
				}
			}
		}
		return static_cast<int>(Intersection::Inside);
	}

	void DrawPoints();
	void DrawLines();
	void DrawPlanes();
	void DrawNormals();

private:
	Plane4 m_planes[6];
	// near top/bottom left/right
	glm::vec3 ntl, ntr, nbl, nbr;
	// far top/bottom left/right
	glm::vec3 ftl, ftr, fbl, fbr;
	// near/far distance, aspect ratio, fov angle, tangent something
	float near, far, ratio, angle, tang;
	// near/far width, near/far height
	float nw, nh, fw, fh;


	unsigned int vao = 0, vbo = 0, ebo = 0;
};

