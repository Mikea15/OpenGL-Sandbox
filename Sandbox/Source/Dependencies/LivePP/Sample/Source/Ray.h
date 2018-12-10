
#pragma once

#include "Vec.h"


struct Ray
{
	Vec origin;
	Vec direction;

	inline Ray(Vec o_, Vec d_)
		: origin(o_)
		, direction(d_)
	{
	}
};
