#pragma once

#include <gl/glew.h>

#include <random>

#include "../Rendering/Shader.h"
#include "../Rendering/Primitives/Quad.h"

class SSAO
{
public:
	struct Params
	{
		float Intensity = 1.0f;
		int KernelSize = 64;
		float Radius = 0.5f;
		float Bias = 0.025f;

		bool operator==(const Params& rhs) {
			if (Intensity != rhs.Intensity) return false;
			if (KernelSize != rhs.KernelSize) return false;
			if (Radius != rhs.Radius) return false;
			if (Bias != rhs.Bias) return false;
			return true;
		}

		bool operator!=(const Params& rhs) {
			return !(*this == rhs);
		}
	};

	SSAO();
	~SSAO();

	void SetupShaders(Shader ssao, Shader ssaoBlur);
	void GenBuffers(unsigned int width, unsigned int height);
	void Process(const glm::mat4& projection, unsigned int gPosition, unsigned int gNormal);
	void BindTextureMaps();

	Params GetParams() const { return m_params; }
	void SetParams(Params params) { m_params = params; }

	unsigned int GetColorBuffer() const { return ssaoColorBuffer; }
	unsigned int GetColorBufferBlur() const { return ssaoColorBufferBlur; }

private:
	// ssao
	std::vector<glm::vec3> ssaoKernel;
	unsigned int noiseTexture;
	unsigned int ssaoFBO;
	unsigned int ssaoBlurFBO;
	unsigned int ssaoColorBuffer;
	unsigned int ssaoColorBufferBlur;

	Params m_params;

	Shader shaderSSAO;
	Shader shaderSSAOBlur;

	Quad quad;
};

