#include "Primitives.h"

#include <glm/glm.hpp>
#include <gl/glew.h>

Cube Primitives::cube = Cube();
Quad Primitives::quad = Quad();
Sphere Primitives::sphere = Sphere();

unsigned int Primitives::indexCount = 0;
unsigned int Primitives::pointVao = 0;
unsigned int Primitives::pointVbo = 0;


void Primitives::RenderPoint()
{
	if (pointVao == 0)
	{
		float point[3] = { 0.0f, 0.0f, 0.0f };
		// Setup
		glGenVertexArrays(1, &pointVao);
		glGenBuffers(1, &pointVbo);

		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, pointVbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), &point, GL_STATIC_DRAW);

		// link vertex attributes
		glBindVertexArray(pointVao);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}
	glBindVertexArray(pointVao);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
}

void Primitives::RenderCube(bool wireframe, bool instanced, unsigned int count)
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	cube.Draw();

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Primitives::RenderQuad(bool wireframe, bool instanced, unsigned int count)
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (!instanced)
	{
		quad.Draw();
	}
	else
	{
		quad.DrawInstanced(count);
	}

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Primitives::RenderSphere(bool wireframe, bool instanced, unsigned int count)
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (!instanced)
	{
		sphere.Draw();
	}
	else
	{
		sphere.DrawInstanced(count);
	}

	
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
