
#include "QuadTree.h"

QuadTree::QuadTree()
	: QuadTree(glm::vec2(0.0f), 1.0f)
{
}

QuadTree::QuadTree(const glm::vec2& origin, float size)
	: m_position(origin), size(size), m_storePos(0.0f)
	, m_topLeft(nullptr)
	, m_topRight(nullptr)
	, m_bottomLeft(nullptr)
	, m_bottomRight(nullptr)
{
	// point on quadTree plane
	const glm::vec3 ptOnPlane = glm::vec3(m_position.x, 0.0f, m_position.y);
	m_box = BoundingBox(ptOnPlane, size);
}

QuadTree::~QuadTree()
{
}

void QuadTree::Subdivide()
{
	float halfSize = size * 0.5f;
	glm::vec2 topLeftPos		= m_position + glm::vec2(-halfSize, halfSize);
	glm::vec2 topRightPos		= m_position + glm::vec2(halfSize, halfSize);
	glm::vec2 bottomLeftPos		= m_position + glm::vec2(-halfSize, -halfSize);
	glm::vec2 bottomRightPos	= m_position + glm::vec2(halfSize, -halfSize);

	m_topLeft		= std::make_shared<QuadTree>(topLeftPos, halfSize);
	m_topRight		= std::make_shared<QuadTree>(topRightPos, halfSize);
	m_bottomLeft	= std::make_shared<QuadTree>(bottomLeftPos, halfSize);
	m_bottomRight	= std::make_shared<QuadTree>(bottomRightPos, halfSize);
}

bool QuadTree::Insert(const glm::vec3& pos)
{
	// use on the QuadTree Plane
	const glm::vec3 p = glm::vec3(pos.x, 0.0f, pos.z);
	if( !m_box.Intersect(p) )
	{
		return false;
	}

	if (m_storePos == glm::vec3(0.0f) && m_topLeft == nullptr)
	{
		m_storePos = p;
		return true;
	}

	if (m_topLeft == nullptr)
	{
		Subdivide();
	}

	if (m_topLeft->Insert(p)) return true;
	if (m_topRight->Insert(p)) return true;
	if (m_bottomLeft->Insert(p)) return true;
	if (m_bottomRight->Insert(p)) return true;

	return false;
}

void QuadTree::Search(BoundingBox range, std::vector<glm::vec3>& outResult)
{
	if (!m_box.Intersect(range))
	{
		return;
	}

	// check objects at this bounds level#
	if (range.Intersect(glm::vec3(m_storePos.x, m_position.y, m_storePos.z))) 
	{
		outResult.push_back(m_storePos);
	}

	if (m_topLeft == nullptr)
	{
		return;
	}

	m_topLeft->Search(range, outResult);
	m_topRight->Search(range, outResult);
	m_bottomLeft->Search(range, outResult);
	m_bottomRight->Search(range, outResult);
}

void QuadTree::GetAllBoundingBoxes(std::vector<BoundingBox>& outResult)
{
	outResult.push_back(m_box);

	if (m_topLeft == nullptr) return;

	m_topLeft->GetAllBoundingBoxes(outResult);
	m_topRight->GetAllBoundingBoxes(outResult);
	m_bottomLeft->GetAllBoundingBoxes(outResult);
	m_bottomRight->GetAllBoundingBoxes(outResult);
}

