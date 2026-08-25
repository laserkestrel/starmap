// GalaxyQuadTree.h
#pragma once // Header guard

#include <SFML/Graphics.hpp>    // Include necessary headers
#include "GalaxyQuadTreeNode.h" // Include the node structure
#include "Star.h"
#include <vector>

// GalaxyQuadTree
// - Manages the quadtree root node and the spatial boundary used for all
//   insert/query operations.
// - The constructor takes an initial boundary parameter which is assigned
//   to the `boundary` member; the root node is then created using that
//   initialized member. This avoids using an uninitialized boundary when
//   constructing the root node.
class GalaxyQuadTree
{
public:
	GalaxyQuadTree(const sf::FloatRect &boundary_, int capacity); // Constructor
	// Set pointer to the canonical star vector used by the quadtree nodes.
	void setStarVector(const std::vector<Star> *sv);
	// Insert a star by index into the quadtree.
	// Returns false if the star lies outside the tree boundary and was not stored --
	// callers should check this rather than assuming every star made it in.
	bool insert(size_t starIndex);
	std::vector<Star> query(const sf::Vector2f &point, float radius);
	GalaxyQuadTreeNode *getRootNode() const
	{
		return root;
	} // Query stars within a radius around a point
	void debugPrint() const;

private:
	GalaxyQuadTreeNode *root; // Pointer to the root node of the quadtree
	int capacity;              // Maximum capacity of stars in a node before splitting
	sf::FloatRect boundary;    // Spatial bounds for the entire quadtree
	const std::vector<Star> *starVec; // Pointer to canonical star storage (set via setStarVector)
};