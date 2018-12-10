#pragma once

#include <vector>

#include "Primitives/Quad.h"
#include "Primitives/Cube.h"

class Primitives
{
public:
	static void RenderCube();
	static void RenderQuad();
	static void RenderSphere();

	static Cube cube;
	static Quad quad;

	static unsigned int indexCount;
	static unsigned int sphereVAO;
};