
#pragma once

#include <stdint.h>


namespace random
{
	static const double XOR_SHIFT_ONE_BY_MAX_UINT32 = 1.0 / static_cast<double>(static_cast<uint32_t>(-1));

	static uint32_t xorShift_x = 123456789u;
	static uint32_t xorShift_y = 362436069u;
	static uint32_t xorShift_z = 521288629u;
	static uint32_t xorShift_w = 88675123u;

	inline uint32_t XorShift(void)
	{
		const uint32_t t = xorShift_x ^ (xorShift_x << 11);
		xorShift_x = xorShift_y;
		xorShift_y = xorShift_z;
		xorShift_z = xorShift_w;
		xorShift_w ^= (xorShift_w >> 19) ^ (t ^ (t >> 8));

		return xorShift_w;
	}


	inline uint32_t XorShift(uint32_t minValue, uint32_t maxValue)
	{
		const uint32_t range = maxValue - minValue;
		const uint32_t randomNumber = XorShift();
		return ((randomNumber % range) + minValue);
	}


	inline int32_t XorShift(int32_t minValue, int32_t maxValue)
	{
		const uint32_t range = static_cast<uint32_t>(maxValue - minValue);
		const uint32_t randomNumber = XorShift();
		return (static_cast<int32_t>(randomNumber % range) + minValue);
	}


	inline double XorShift(double minValue, double maxValue)
	{
		const double range = maxValue - minValue;
		const uint32_t randomNumber = XorShift();
		return (static_cast<double>(randomNumber) * range * XOR_SHIFT_ONE_BY_MAX_UINT32) + minValue;
	}
}
