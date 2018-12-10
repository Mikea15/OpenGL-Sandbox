
#pragma once

#include "Vec.h"
#include "ReflectionType.h"
#include "Ray.h"


struct Sphere
{
	double radius;
	double position[3];
	double emission[3];
	double color[3];
	ReflectionType::Enum reflectionType;
	bool isLight;

	inline Vec GetPosition(void) const
	{
		return Vec(position[0], position[1], position[2]);
	}

	inline Vec GetEmission(void) const
	{
		return Vec(emission[0], emission[1], emission[2]);
	}

	inline Vec GetColor(void) const
	{
		return Vec(color[0], color[1], color[2]);
	}

	// returns distance, 0 if nothing was hit
	inline double intersect(const Ray& r) const
	{
		// Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
		const Vec op = GetPosition() - r.origin;

		// Live++: try changing the epsilon value to something larger and see what happens
		const double eps = 0.00001;
		const double b = op.dot(r.direction);

		const double det = b * b - op.dot(op) + radius * radius;
		if (det < 0)
		{
			return 0;
		}

		{
			const double t  = b - sqrt(det);
			if (t > eps)
			{
				return t;
			}
		}
		{
			const double t  = b + sqrt(det);
			if (t > eps)
			{
				return t;
			}
		}

		return 0;
	}
};
