
#pragma once

#include "Vec.h"

struct Ray;


namespace scene
{
	Vec TraceRay(const Ray& r, int depth, int E = 1);
}
