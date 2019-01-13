#pragma once

#include <iostream>

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


#include <gl/glew.h>
#include "Dependencies/std_image/stb_image.h"

namespace MathUtils
{
	static float Lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}
}
