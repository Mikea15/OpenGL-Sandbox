#include "BoundingFrustum.h"

#include <vector>

#include "Plane.h"
#include "BoundingBox.h"

BoundingFrustum::BoundingFrustum()
{}

BoundingFrustum::BoundingFrustum(const glm::mat4 & viewProjection)
{
	SetViewProjectionMatrix(viewProjection);
}

BoundingFrustum::~BoundingFrustum()
{}

void BoundingFrustum::SetViewProjectionMatrix(const glm::mat4 & viewProjection)
{
	viewProj = viewProjection;
	CreatePlanes();
	CreateCorners();
}

bool BoundingFrustum::Intersects(const BoundingBox& aabb) const
{
	return Contains(aabb) != ContainmentType::Disjoint;
}

bool BoundingFrustum::Intersects(const BoundingFrustum& frustum) const
{
	return Contains(frustum) != ContainmentType::Disjoint;
}

PlaneIntersectionType BoundingFrustum::Intersects(const Plane& plane) const
{
	PlaneIntersectionType pit = plane.Intersects(corners[0]);
	for (int i = 1; i < corners.size(); ++i)
	{
		if (plane.Intersects(corners[i]) != pit)
		{
			pit = PlaneIntersectionType::Intersecting;
		}
	}
	return pit;
}

ContainmentType BoundingFrustum::Contains(const glm::vec3& point) const
{
	for (unsigned int i = 0; i < 6; ++i)
	{
		if (PlaneHelper::ClassifyPoint(point, planes[i]) > 0)
		{
			return ContainmentType::Disjoint;
		}
	}
	return ContainmentType::Contains;
}

ContainmentType BoundingFrustum::Contains(const BoundingBox& aabb) const
{
	bool intersects = false;
	for (unsigned int i = 0; i < 6; ++i)
	{
		PlaneIntersectionType planeIntersectionType = aabb.Intersects(planes[i]);
		switch (planeIntersectionType)
		{
		case PlaneIntersectionType::Front:
			return ContainmentType::Disjoint;
		case PlaneIntersectionType::Intersecting:
			intersects = true;
			break;
		}
	}

	return intersects ? ContainmentType::Intersects : ContainmentType::Contains;
}

ContainmentType BoundingFrustum::Contains(const BoundingFrustum& frustum) const
{
	bool intersects = false;
	for (unsigned int i = 0; i < 6; ++i)
	{
		PlaneIntersectionType planeIntersectionType = frustum.Intersects(planes[i]);
		switch (planeIntersectionType)
		{
		case PlaneIntersectionType::Front:
			return ContainmentType::Disjoint;
		case PlaneIntersectionType::Intersecting:
			intersects = true;
			break;
		}
	}
	return intersects ? ContainmentType::Intersects : ContainmentType::Contains;

	return ContainmentType();
}

void BoundingFrustum::CreateCorners()
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

void BoundingFrustum::CreatePlanes()
{
	planes.resize(6);

	planes[0] = Plane(-viewProj[0][2], -viewProj[1][2], -viewProj[2][2], -viewProj[3][2]);
	planes[1] = Plane(viewProj[0][2] - viewProj[0][3], viewProj[1][2] - viewProj[1][3], viewProj[2][2] - viewProj[2][3], viewProj[3][2] - viewProj[3][3]);
	planes[2] = Plane(-viewProj[0][3] - viewProj[0][0], -viewProj[1][3] - viewProj[1][0], -viewProj[2][3] - viewProj[2][0], -viewProj[3][3] - viewProj[3][0]);
	planes[3] = Plane(viewProj[0][0] - viewProj[0][3], viewProj[1][0] - viewProj[1][3], viewProj[2][0] - viewProj[2][3], viewProj[3][0] - viewProj[3][3]);
	planes[4] = Plane(viewProj[0][1] - viewProj[0][3], viewProj[1][1] - viewProj[1][3], viewProj[2][1] - viewProj[2][3], viewProj[3][1] - viewProj[3][3]);
	planes[5] = Plane(-viewProj[0][3] - viewProj[0][1], -viewProj[1][3] - viewProj[1][1], -viewProj[2][3] - viewProj[2][1], -viewProj[3][3] - viewProj[3][1]);

	planes[0].Normalize();
	planes[1].Normalize();
	planes[2].Normalize();
	planes[3].Normalize();
	planes[4].Normalize();
	planes[5].Normalize();
}

glm::vec3 BoundingFrustum::IntersectionPoint(const Plane & a, const Plane & b, const Plane & c)
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

void BoundingFrustum::DrawPoints()
{
	if (vao == 0)
	{
		std::vector<glm::vec3> points;

		points.push_back(glm::vec3(ntl.x, ntl.y, ntl.z));
		points.push_back(glm::vec3(ntr.x, ntr.y, ntr.z));
		points.push_back(glm::vec3(nbl.x, nbl.y, nbl.z));
		points.push_back(glm::vec3(nbr.x, nbr.y, nbr.z));
		points.push_back(glm::vec3(ftl.x, ftl.y, ftl.z));
		points.push_back(glm::vec3(ftr.x, ftr.y, ftr.z));
		points.push_back(glm::vec3(fbl.x, fbl.y, fbl.z));
		points.push_back(glm::vec3(fbr.x, fbr.y, fbr.z));

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), &points[0], GL_STATIC_DRAW);
		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0); // layout (location = 0)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glBindVertexArray(0);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, 8);
	glBindVertexArray(0);
}


void BoundingFrustum::DrawLines()
{
	//near plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(ntl.x, ntl.y, ntl.z);
	glVertex3f(ntr.x, ntr.y, ntr.z);
	glVertex3f(nbr.x, nbr.y, nbr.z);
	glVertex3f(nbl.x, nbl.y, nbl.z);
	glEnd();

	//far plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(ftr.x, ftr.y, ftr.z);
	glVertex3f(ftl.x, ftl.y, ftl.z);
	glVertex3f(fbl.x, fbl.y, fbl.z);
	glVertex3f(fbr.x, fbr.y, fbr.z);
	glEnd();

	//bottom plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(nbl.x, nbl.y, nbl.z);
	glVertex3f(nbr.x, nbr.y, nbr.z);
	glVertex3f(fbr.x, fbr.y, fbr.z);
	glVertex3f(fbl.x, fbl.y, fbl.z);
	glEnd();

	//top plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(ntr.x, ntr.y, ntr.z);
	glVertex3f(ntl.x, ntl.y, ntl.z);
	glVertex3f(ftl.x, ftl.y, ftl.z);
	glVertex3f(ftr.x, ftr.y, ftr.z);
	glEnd();

	//left plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(ntl.x, ntl.y, ntl.z);
	glVertex3f(nbl.x, nbl.y, nbl.z);
	glVertex3f(fbl.x, fbl.y, fbl.z);
	glVertex3f(ftl.x, ftl.y, ftl.z);
	glEnd();

	// right plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(nbr.x, nbr.y, nbr.z);
	glVertex3f(ntr.x, ntr.y, ntr.z);
	glVertex3f(ftr.x, ftr.y, ftr.z);
	glVertex3f(fbr.x, fbr.y, fbr.z);
	glEnd();
}


void BoundingFrustum::DrawPlanes()
{
	// if (vao == 0) 
	{
		float vertices[] = {
			ftr.x, ftr.y, ftr.z,  // top right
			fbr.x, fbr.y, fbr.z,  // bottom right
			fbl.x, fbl.y, fbl.z,  // bottom left
			ftl.x, ftl.y, ftl.z,  // top left 
		};
		std::vector<glm::vec3> points;
		//near plane
		points.push_back(glm::vec3(ntl.x, ntl.y, ntl.z));
		points.push_back(glm::vec3(ntr.x, ntr.y, ntr.z));
		points.push_back(glm::vec3(nbr.x, nbr.y, nbr.z));
		points.push_back(glm::vec3(nbl.x, nbl.y, nbl.z));
		//far plane
		points.push_back(glm::vec3(ftr.x, ftr.y, ftr.z));
		points.push_back(glm::vec3(ftl.x, ftl.y, ftl.z));
		points.push_back(glm::vec3(fbl.x, fbl.y, fbl.z));
		points.push_back(glm::vec3(fbr.x, fbr.y, fbr.z));
		//bottom plane
		points.push_back(glm::vec3(nbl.x, nbl.y, nbl.z));
		points.push_back(glm::vec3(nbr.x, nbr.y, nbr.z));
		points.push_back(glm::vec3(fbr.x, fbr.y, fbr.z));
		points.push_back(glm::vec3(fbl.x, fbl.y, fbl.z));
		//top plane
		points.push_back(glm::vec3(ntr.x, ntr.y, ntr.z));
		points.push_back(glm::vec3(ntl.x, ntl.y, ntl.z));
		points.push_back(glm::vec3(ftl.x, ftl.y, ftl.z));
		points.push_back(glm::vec3(ftr.x, ftr.y, ftr.z));
		//left plane
		points.push_back(glm::vec3(ntl.x, ntl.y, ntl.z));
		points.push_back(glm::vec3(nbl.x, nbl.y, nbl.z));
		points.push_back(glm::vec3(fbl.x, fbl.y, fbl.z));
		points.push_back(glm::vec3(ftl.x, ftl.y, ftl.z));
		// right plane
		points.push_back(glm::vec3(nbr.x, nbr.y, nbr.z));
		points.push_back(glm::vec3(ntr.x, ntr.y, ntr.z));
		points.push_back(glm::vec3(ftr.x, ftr.y, ftr.z));
		points.push_back(glm::vec3(fbr.x, fbr.y, fbr.z));

		unsigned int indices[] = {
			0, 1, 3,
			1, 2, 3
		};

		// Setup
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// link vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}



	glBindVertexArray(vao);
	// glDrawArrays(GL_POINTS, 0, 24);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void BoundingFrustum::DrawNormals()
{
	//glm::vec3 a, b;

	//glBegin(GL_LINES);

	//// near
	//a = (ntr + ntl + nbr + nbl) * 0.25f;
	//b = a + m_planes[Near].normal;
	//glVertex3f(a.x, a.y, a.z);
	//glVertex3f(b.x, b.y, b.z);

	//// far
	//a = (ftr + ftl + fbr + fbl) * 0.25f;
	//b = a + m_planes[Far].normal;
	//glVertex3f(a.x, a.y, a.z);
	//glVertex3f(b.x, b.y, b.z);

	//// left
	//a = (ftl + fbl + nbl + ntl) * 0.25f;
	//b = a + m_planes[Left].normal;
	//glVertex3f(a.x, a.y, a.z);
	//glVertex3f(b.x, b.y, b.z);

	//// right
	//a = (ftr + nbr + fbr + ntr) * 0.25f;
	//b = a + m_planes[Right].normal;
	//glVertex3f(a.x, a.y, a.z);
	//glVertex3f(b.x, b.y, b.z);

	//// top
	//a = (ftr + ftl + ntr + ntl) * 0.25f;
	//b = a + m_planes[Top].normal;
	//glVertex3f(a.x, a.y, a.z);
	//glVertex3f(b.x, b.y, b.z);

	//// bottom
	//a = (fbr + fbl + nbr + nbl) * 0.25f;
	//b = a + m_planes[Bottom].normal;
	//glVertex3f(a.x, a.y, a.z);
	//glVertex3f(b.x, b.y, b.z);

	//glEnd();
}