
#pragma once

struct Ray;
struct Vec;


namespace pathTrace
{
	void Trace(const Ray& cameraRay, unsigned int rayCount, unsigned int width, unsigned int height, unsigned int* samplesPerPixel, Vec* colorData, unsigned char* rgbaData);
}
