
#include "Scene.h"
#include "Sphere.h"
#include "XorShift.h"


static const double PI = 3.14159265358979323846264338327950288;
static const double ONE_OVER_PI = 0.318309886183790671538;


// simple scene description
// Live++: try changing any of these values. changing emission, colors and materials leads to interesting effects
static const unsigned int SPHERE_COUNT = 9u;
static const Sphere g_spheres[SPHERE_COUNT] =
{
//	radius		position					emission			color						material					isLight
	{ 1e5,		{ 1e5 + 1, 40.8, 81.6 },	{ 0, 0, 0 },		{ 0.75, 0.25, 0.25 },		ReflectionType::DIFFUSE,	false },	// Left
	{ 1e5,		{ -1e5 + 99, 40.8, 81.6 },	{ 0, 0, 0 },		{ 0.25, 0.25, 0.75 },		ReflectionType::DIFFUSE,	false },	// Right
	{ 1e5,		{ 50, 40.8, 1e5 },			{ 0, 0, 0 },		{ 0.75, 0.75, 0.75 },		ReflectionType::DIFFUSE,	false },	// Back
	{ 1e5,		{ 50, 40.8, -1e5 + 170 },	{ 0, 0, 0 },		{ 0.00, 0.50, 0.00 },		ReflectionType::DIFFUSE,	false },	// Front
	{ 1e5,		{ 50, 1e5, 81.6 },			{ 0, 0, 0 },		{ 0.75, 0.75, 0.75 },		ReflectionType::DIFFUSE,	false },	// Bottom
	{ 1e5,		{ 50, -1e5 + 81.6, 81.6 },	{ 0, 0, 0 },		{ 0.75, 0.75, 0.75 },		ReflectionType::DIFFUSE,	false },	// Top
	{ 16.5,		{ 27, 16.5, 47 },			{ 0, 0, 0 },		{ 0.999, 0.999, 0.999 },	ReflectionType::SPECULAR,	false },	// Mirror
	{ 16.5,		{ 73, 16.5, 78 },			{ 0, 0, 0 },		{ 0.999, 0.999, 0.999 },	ReflectionType::REFRACTIVE,	false },	// Glass
	{ 1.5,		{ 50, 81.6 - 16.5, 81.6 },	{ 400, 400, 400 },	{ 0, 0, 0 },				ReflectionType::DIFFUSE,	true }		// Light
};


static inline bool intersect(const Ray& r, double& t, int& id)
{
	double inf = t = 1e20;
	for (unsigned int i=0u; i < SPHERE_COUNT; ++i)
	{
		double d = g_spheres[i].intersect(r);
		if (d && d < t)
		{
			t = d;
			id = i;
		}
	}

	return t < inf;
}


Vec scene::TraceRay(const Ray& r, int depth, int E)
{
	double t = 0; // distance to intersection
	int id = 0; // id of intersected object
	if (!intersect(r, t, id))
	{
		// if miss, return black
		return Vec();
	}

	// the hit object
	const Sphere& obj = g_spheres[id];
	const Vec x = r.origin + r.direction * t;
	const Vec n = (x - obj.GetPosition()).normalize();
	const Vec nl = n.dot(r.direction) < 0 ? n : n * -1;
	Vec f = obj.GetColor();

	// max refl
	const double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z;
	if (++depth > 5 || !p)
	{
		if (random::XorShift(0.0f, 1.0f) < p)
		{
			f = f * (1 / p);
		}
		else
		{
			return obj.GetEmission() * E;
		}
	}

	// Live++: try changing the max. iteration depth
	// (=0  : only lights)
	// (=1  : only diffuse, no color bleeding)
	// (>=2 : multi-bounce GI, reflections, ...)
	if (depth > 5)
	{
		return obj.GetEmission() * E;
	}

	// ideal DIFFUSE reflection
	if (obj.reflectionType == ReflectionType::DIFFUSE)
	{
		double r1 = 2 * PI * random::XorShift(0.0f, 1.0f);
		double r2 = random::XorShift(0.0f, 1.0f);
		double r2s = sqrt(r2);

		Vec w = nl;
		Vec u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)).cross(w)).normalize();
		Vec v = w.cross(u);
		Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalize();

		// loop over any lights
		Vec e;
		for (int i = 0; i < SPHERE_COUNT; i++)
		{
			const Sphere& s = g_spheres[i];

			// skip non-lights
			if (!s.isLight)
			{
				continue;
			}

			Vec sw = s.GetPosition() - x;
			Vec su = ((fabs(sw.x) > .1 ? Vec(0, 1) : Vec(1)).cross(sw)).normalize();
			Vec sv = sw.cross(su);
			double cos_a_max = sqrt(1 - s.radius * s.radius / (x - s.GetPosition()).dot(x - s.GetPosition()));
			double eps1 = random::XorShift(0.0f, 1.0f), eps2 = random::XorShift(0.0f, 1.0f);
			double cos_a = 1 - eps1 + eps1 * cos_a_max;
			double sin_a = sqrt(1 - cos_a * cos_a);
			double phi = 2 * PI * eps2;
			Vec l = su * cos(phi) * sin_a + sv * sin(phi) * sin_a + sw * cos_a;
			l = l.normalize();

			// shadow ray
			if (intersect(Ray(x, l), t, id) && id == i)
			{
				double omega = 2 * PI * (1 - cos_a_max);

				// 1/pi for BRDF
				e = e + f.mult(s.GetEmission() * l.dot(nl) * omega) * ONE_OVER_PI;
			}
		}

		return obj.GetEmission() * E + e + f.mult(TraceRay(Ray(x, d), depth, 0));
	}
	// Ideal SPECULAR reflection
	else if (obj.reflectionType == ReflectionType::SPECULAR)
	{
		return obj.GetEmission() + f.mult(TraceRay(Ray(x, r.direction - n * 2 * n.dot(r.direction)), depth));
	}

	// Ideal dielectric REFRACTION
	Ray reflRay(x, r.direction - n * 2 * n.dot(r.direction));

	// Ray from outside going in?
	bool into = n.dot(nl) > 0;
	double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.direction.dot(nl), cos2t;

	// Total internal reflection
	if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)
	{
		return obj.GetEmission() + f.mult(TraceRay(reflRay, depth));
	}

	Vec tdir = (r.direction * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalize();
	double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(n));
	double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);

	// Russian roulette
	return obj.GetEmission() + f.mult(depth > 2 ? (random::XorShift(0.0f, 1.0f) < P ?
		TraceRay(reflRay, depth) * RP
		: TraceRay(Ray(x, tdir), depth) * TP)
		: TraceRay(reflRay, depth) * Re + TraceRay(Ray(x, tdir), depth) * Tr);
}
