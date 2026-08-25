// GalaxyQuadTree.h
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include "GalaxyQuadTreeNode.h"
#include "Star.h"
#include <memory>
#include <vector>

// Spatial index over the star catalogue, in world units (parsecs).
//
// The boundary is set from the catalogue's own extent, not from the window, so
// a star cannot fall outside the tree just because it is off-screen.
class GalaxyQuadTree
{
public:
	GalaxyQuadTree(const sf::FloatRect &worldBoundary, int capacity);

	// Point the tree at the canonical star vector. Must be called before insert().
	void setStarVector(const std::vector<Star> *sv);

	// Insert a star by index. Returns false only if the star lies outside the
	// tree's world boundary -- callers should check rather than assume.
	bool insert(size_t starIndex);

	const GalaxyQuadTreeNode *getRootNode() const { return root.get(); }
	const sf::FloatRect &getBoundary() const { return boundary; }
	void debugPrint() const;

private:
	std::unique_ptr<GalaxyQuadTreeNode> root; // owns the whole tree
	int capacity;
	sf::FloatRect boundary;
	const std::vector<Star> *starVec = nullptr;
};
