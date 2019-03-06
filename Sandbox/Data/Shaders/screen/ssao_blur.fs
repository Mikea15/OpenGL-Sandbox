#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main() 
{
	int kernelSize = 2;
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));

    float result = 0.0;
    for (int x = -kernelSize; x < kernelSize; ++x) 
    {
        for (int y = -kernelSize; y < kernelSize; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }

    FragColor = result / (kernelSize * 2.0 * kernelSize * 2.0f);
}
