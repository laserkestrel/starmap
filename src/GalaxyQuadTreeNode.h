// GalaxyQuadTreeNode.h
#pragma once

#include <SFML/Graphics.hpp>
#include "Star.h"
#include <vector>

struct GalaxyQuadTreeNode
{
	sf::FloatRect boundary;
	// store indices into the canonical star vector owned by Game
	std::vector<size_t> starIndices;
	const std::vector<Star> *starVec; // pointer to canonical star storage
	GalaxyQuadTreeNode *children[4];
	bool isLeaf;
	int capacity;

	GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity, const std::vector<Star> *starVecPtr);
	GalaxyQuadTreeNode *getChild(int index) const;
	bool insert(size_t starIndex);
	void split();
	void debugPrint(int depth = 0) const;
};