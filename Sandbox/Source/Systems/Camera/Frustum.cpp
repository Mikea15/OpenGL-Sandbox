#include "Frustum.h"

#include <vector>

void Frustum::DrawPoints()
{
	glBegin(GL_POINTS);
	glVertex3f(ntl.x, ntl.y, ntl.z);
	glVertex3f(ntr.x, ntr.y, ntr.z);
	glVertex3f(nbl.x, nbl.y, nbl.z);
	glVertex3f(nbr.x, nbr.y, nbr.z);

	glVertex3f(ftl.x, ftl.y, ftl.z);
	glVertex3f(ftr.x, ftr.y, ftr.z);
	glVertex3f(fbl.x, fbl.y, fbl.z);
	glVertex3f(fbr.x, fbr.y, fbr.z);
	glEnd();
}


void Frustum::DrawLines() 
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


void Frustum::DrawPlanes() 
{
	if (vao == 0) 
	{
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
		// Setup
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &points[0], GL_STATIC_DRAW);

		// link vertex attributes
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Frustum::DrawNormals() 
{
	glm::vec3 a, b;

	glBegin(GL_LINES);

	// near
	a = (ntr + ntl + nbr + nbl) * 0.25f;
	b = a + m_planes[Near].normal;
	glVertex3f(a.x, a.y, a.z);
	glVertex3f(b.x, b.y, b.z);

	// far
	a = (ftr + ftl + fbr + fbl) * 0.25f;
	b = a + m_planes[Far].normal;
	glVertex3f(a.x, a.y, a.z);
	glVertex3f(b.x, b.y, b.z);

	// left
	a = (ftl + fbl + nbl + ntl) * 0.25f;
	b = a + m_planes[Left].normal;
	glVertex3f(a.x, a.y, a.z);
	glVertex3f(b.x, b.y, b.z);

	// right
	a = (ftr + nbr + fbr + ntr) * 0.25f;
	b = a + m_planes[Right].normal;
	glVertex3f(a.x, a.y, a.z);
	glVertex3f(b.x, b.y, b.z);

	// top
	a = (ftr + ftl + ntr + ntl) * 0.25f;
	b = a + m_planes[Top].normal;
	glVertex3f(a.x, a.y, a.z);
	glVertex3f(b.x, b.y, b.z);

	// bottom
	a = (fbr + fbl + nbr + nbl) * 0.25f;
	b = a + m_planes[Bottom].normal;
	glVertex3f(a.x, a.y, a.z);
	glVertex3f(b.x, b.y, b.z);

	glEnd();
}