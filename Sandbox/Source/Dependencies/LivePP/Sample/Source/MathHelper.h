
#pragma once

#include <math.h>


namespace math
{
	inline double Clamp(double x)
	{
		return x < 0 ? 0 : x > 1 ? 1 : x;
	}

	// crude gamma approximation
	inline int Gamma(double x)
	{
		const double clamped = Clamp(x);

		// Live++: try changing this formula, e.g. increasing the scale to 1000 to overflow pixel values.
		// when no sync point is installed, you will see that some pixels have been updated with the old code,
		// while others have been updated with the new code.
		const double scale = 255;

		return static_cast<int>(0.5 + scale * 1.138 * sqrt(clamped) - scale * 0.138 * clamped);
	}
}
