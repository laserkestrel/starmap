// GalaxyQuadTree.cpp
#include "GalaxyQuadTree.h"

GalaxyQuadTree::GalaxyQuadTree(const sf::FloatRect &worldBoundary, int capacity)
	: capacity(capacity), boundary(worldBoundary)
{
	root = std::make_unique<GalaxyQuadTreeNode>(boundary, capacity, starVec, 0);
}

void GalaxyQuadTree::setStarVector(const std::vector<Star> *sv)
{
	starVec = sv;
	if (root)
	{
		root->setStarVector(starVec);
	}
}

bool GalaxyQuadTree::insert(size_t starIndex)
{
	if (!root)
	{
		root = std::make_unique<GalaxyQuadTreeNode>(boundary, capacity, starVec, 0);
	}
	return root->insert(starIndex);
}

void GalaxyQuadTree::queryRange(const sf::FloatRect &area, std::vector<size_t> &out) const
{
	if (root)
	{
		root->queryRange(area, out);
	}
}

void GalaxyQuadTree::debugPrint() const
{
	if (root)
	{
		root->debugPrint();
	}
}
