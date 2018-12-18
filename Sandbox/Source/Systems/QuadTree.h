#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "Entity.h"
#include "Physics/BoundingBox.h"



class QuadTree
{
public:
	QuadTree();
	QuadTree(const glm::vec2& origin, float size);
	~QuadTree();

	void Subdivide();
	bool Insert(const glm::vec3& pos);
	void Search(BoundingBox range, std::vector<glm::vec3>& outResult);

	void GetAllBoundingBoxes(std::vector<BoundingBox>& outResult);

private:
	glm::vec2 m_position;
	float size;

	BoundingBox m_box;

	glm::vec3 m_storePos;

	std::shared_ptr<QuadTree> m_parent;

	std::shared_ptr<QuadTree> m_topLeft;
	std::shared_ptr<QuadTree> m_topRight;
	std::shared_ptr<QuadTree> m_bottomLeft;
	std::shared_ptr<QuadTree> m_bottomRight;
};

