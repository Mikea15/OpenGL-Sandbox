
#include "Update.h"
#include "XorShift.h"
#include "MathHelper.h"
#include "Vec.h"
#include "Scene.h"
#include "Ray.h"


void pathTrace::Trace(const Ray& cameraRay, unsigned int rayCount, unsigned int width, unsigned int height, unsigned int* samplesPerPixel, Vec* colorData, unsigned char* rgbaData)
{
	// Live++: try changing these to distort the camera
	const Vec cx = Vec(width * 0.5135 / height);
	const Vec cy = (cx.cross(cameraRay.direction)).normalize() * 0.5135;

	for (unsigned int rays=0; rays < rayCount; ++rays)
	{
		// shoot a random ray
		const unsigned int x = random::XorShift(0u, width);
		const unsigned int y = random::XorShift(0u, height);

		const unsigned int index = (height-y-1)*width + x;
		++samplesPerPixel[index];

		for (unsigned int sy=0; sy < 2u; sy++)
		{
			for (unsigned int sx=0; sx < 2u; sx++)
			{
				const double r1 = 2*random::XorShift(0.0f, 1.0f);
				const double dx = r1 < 1
					? sqrt(r1) - 1
					: 1 - sqrt(2-r1);

				const double r2 = 2*random::XorShift(0.0f, 1.0f);
				const double dy = r2 < 1
					? sqrt(r2) - 1
					: 1-sqrt(2-r2);

				Vec d =
					cx*(((sx + 0.5 + dx) * 0.5 + x) / width - 0.5) +
					cy*(((sy + 0.5 + dy) * 0.5 + y) / height - 0.5) +
					cameraRay.direction;

				// camera rays are pushed forward to start in interior
				// Live++: what if we don't do this?
				const Vec r = scene::TraceRay(Ray(cameraRay.origin + d * 130, d.normalize()), 0);

				colorData[index] = colorData[index] + Vec(math::Clamp(r.x)*0.25, math::Clamp(r.y)*0.25, math::Clamp(r.z)*0.25);
			}
		}

		const float oneBySampleCount = 1.0f / samplesPerPixel[index];

		// convert BGR to RGB
		rgbaData[index*4u + 2u] = math::Gamma(colorData[index].x * oneBySampleCount);
		rgbaData[index*4u + 1u] = math::Gamma(colorData[index].y * oneBySampleCount);
		rgbaData[index*4u + 0u] = math::Gamma(colorData[index].z * oneBySampleCount);
	}
}
