#pragma once // Header guard

#include <SFML/Graphics.hpp>	// Include necessary headers
#include "GalaxyQuadTreeNode.h" // Include the node structure

class GalaxyQuadTree
{
public:
	GalaxyQuadTree(const sf::FloatRect &boundary, int capacity);	  // Constructor
	void insert(const Star &star);									  // Insert a star into the quadtree
	std::vector<Star> query(const sf::Vector2f &point, float radius); // Query stars within a radius around a point

private:
	GalaxyQuadTreeNode *root; // Pointer to the root node of the quadtree
	int capacity;			  // Maximum capacity of stars in a node before splitting
							  // Other private helper methods for insertion, splitting nodes, querying, etc.
};