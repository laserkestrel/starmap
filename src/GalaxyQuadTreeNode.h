// GalaxyQuadTreeNode.h
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include "Star.h"
#include <memory>
#include <vector>

// One node of the quadtree.
//
// The boundary is in WORLD units (parsecs, on the x/y plane) -- it used to be
// the window rectangle in pixels, which meant any star that happened to fall
// off-screen was silently absent from the tree and unreachable by every probe.
//
// The tree is 2D over x and y while stars and probes live in 3D. That is still
// correct for the nearest-star search: a star within 3D distance r of a point
// necessarily has an x/y separation of at most r, so a 2D query of radius r
// returns a superset of the 3D matches. The exact 3D distance is then used to
// pick the winner. An octree would prune harder, but this keeps the structure
// simple and never misses a star.
struct GalaxyQuadTreeNode
{
	// Splitting stops here regardless of capacity. Without it, stars sharing a
	// position split forever until the rectangle degenerates and every child
	// rejects them -- at which point they are dropped without a word.
	static constexpr int MAX_DEPTH = 24;

	sf::FloatRect boundary;           // world parsecs
	std::vector<size_t> starIndices;  // indices into the canonical star vector
	const std::vector<Star> *starVec = nullptr;
	std::unique_ptr<GalaxyQuadTreeNode> children[4];
	bool isLeaf = true;
	int capacity = 128;
	int depth = 0;

	GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity,
					   const std::vector<Star> *starVecPtr, int nodeDepth = 0);

	const GalaxyQuadTreeNode *getChild(int index) const;
	bool insert(size_t starIndex);
	void split();
	void setStarVector(const std::vector<Star> *sv);
	void debugPrint(int indentDepth = 0) const;
};
