// GalaxyQuadTreeNode.h
#pragma once

#include <SFML/Graphics.hpp>
#include "Star.h"
#include <vector>

struct GalaxyQuadTreeNode
{
	sf::FloatRect boundary;
	std::vector<Star> stars;
	GalaxyQuadTreeNode *children[4];
	bool isLeaf;
	int capacity;

	GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity);
	GalaxyQuadTreeNode *getChild(int index) const;
	bool insert(const Star &star);
	void split();
	void debugPrint(int depth = 0) const;
};