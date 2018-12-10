
#pragma once

#include <math.h>


struct Vec
{
	double x, y, z;

	inline Vec(double x_ = 0, double y_ = 0, double z_ = 0)
	{
		x = x_;
		y = y_;
		z = z_;
	}

	inline Vec operator+(const Vec& other) const
	{
		return Vec(x + other.x, y + other.y, z + other.z);
	}

	inline Vec operator-(const Vec& other) const
	{
		return Vec(x - other.x, y - other.y, z - other.z);
	}

	inline Vec operator*(double scalar) const
	{
		return Vec(x * scalar, y * scalar, z * scalar);
	}

	inline Vec mult(const Vec& other) const
	{
		return Vec(x * other.x, y * other.y, z * other.z);
	}

	inline Vec normalize(void) const
	{
		const double length = sqrt(x * x + y * y + z * z);
		const double oneByLength = 1 / length;

		return Vec(x*oneByLength, y*oneByLength, z*oneByLength);
	}

	inline double dot(const Vec& b) const
	{
		return x * b.x + y * b.y + z * b.z;
	}

	inline Vec cross(const Vec& other) const
	{
		return Vec(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}
};
