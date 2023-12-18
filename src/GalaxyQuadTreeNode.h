#pragma once // Header guard

#include <SFML/Graphics.hpp> // Include necessary headers
#include "Star.h"			 // Include the Star class or necessary data structure for stars

struct GalaxyQuadTreeNode
{
	sf::FloatRect boundary;			 // Represents the node's boundary in 2D space
	std::vector<Star> stars;		 // Stars within this node's boundary
	GalaxyQuadTreeNode *children[4]; // Pointers to the four children nodes
	bool isLeaf;					 // Flag to indicate if the node is a leaf (has no children)
	// Other necessary attributes and methods
};
