#pragma once

namespace MathUtils
{
	static float Lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}
}
