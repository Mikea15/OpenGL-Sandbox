#pragma once

class Cube
{
public:
	Cube();

	void Init();
	void Draw();

private:
	unsigned int m_VAO;
	unsigned int m_VBO;
};