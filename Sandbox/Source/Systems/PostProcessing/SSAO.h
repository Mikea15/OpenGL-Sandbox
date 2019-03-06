#pragma once

#include <gl/glew.h>

#include <random>

#include "Systems/Rendering/Shader.h"
#include "Systems/Rendering/Primitives.h"

class ShaderManager;

class SSAO
{
public:
	struct Params
	{
		float Intensity = 1.0f;
		int KernelSize = 4;
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

	void LoadShaders(ShaderManager& shaderManager);
	void GenBuffers(unsigned int width, unsigned int height);
	void Process(const glm::mat4& projection, unsigned int gPosition, unsigned int gNormal);
	void BindTextureMaps();

	Params GetParams() const { return m_params; }
	void SetParams(Params params) { m_params = params; }

	unsigned int GetColorBuffer() const { return m_colorBuffer; }
	unsigned int GetColorBufferBlur() const { return m_colorBlurBuffer; }

private:
	// ssao
	std::vector<glm::vec3> ssaoKernel;

	unsigned int m_FBO;
	unsigned int m_blurFBO;
	unsigned int m_colorBuffer;
	unsigned int m_colorBlurBuffer;
	unsigned int m_noiseTexture;

	Params m_params;

	Shader shaderSSAO;
	Shader shaderSSAOBlur;
};

