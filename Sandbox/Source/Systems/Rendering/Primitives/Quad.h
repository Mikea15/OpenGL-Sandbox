#pragma once

class Quad
{
public:
	Quad();

	void Init();
	void Draw();

private:
	unsigned int m_VAO;
	unsigned int m_VBO;
};