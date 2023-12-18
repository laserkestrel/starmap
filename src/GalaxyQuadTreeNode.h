// GalaxyQuadTreeNode.h
#pragma once // Header guard

#include <SFML/Graphics.hpp> // Include necessary headers
#include "Star.h"			 // Include the Star class or necessary data structure for stars

struct GalaxyQuadTreeNode
{
	sf::FloatRect boundary;			 // Represents the node's boundary in 2D space
	std::vector<Star> stars;		 // Stars within this node's boundary
	GalaxyQuadTreeNode *children[4]; // Pointers to the four children nodes
	bool isLeaf;
	int capacity; // Flag to indicate if the node is a leaf (has no children)
	GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity)
		: boundary(nodeBoundary), isLeaf(true)
	{
		for (int i = 0; i < 4; ++i)
		{
			children[i] = nullptr;
		}
	}

	GalaxyQuadTreeNode *getChild(int index) const
	{
		if (index >= 0 && index < 4)
		{
			return children[index];
		}
		return nullptr;
	}

	bool insert(const Star &star)
	{
		if (!boundary.contains(star.getX(), star.getY()))
		{
			return false; // Star doesn't fit in this node's boundary
		}

		if (isLeaf && stars.size() < capacity)
		{
			stars.push_back(star);
			return true;
		}

		if (isLeaf)
		{
			split();
		}

		for (int i = 0; i < 4; ++i)
		{
			if (children[i]->insert(star))
			{
				return true;
			}
		}
		return false;
	}

	void split()
	{
		float subWidth = boundary.width / 2.0f;
		float subHeight = boundary.height / 2.0f;
		float x = boundary.left;
		float y = boundary.top;

		children[0] = new GalaxyQuadTreeNode(sf::FloatRect(x + subWidth, y, subWidth, subHeight), capacity);
		children[1] = new GalaxyQuadTreeNode(sf::FloatRect(x, y, subWidth, subHeight), capacity);
		children[2] = new GalaxyQuadTreeNode(sf::FloatRect(x, y + subHeight, subWidth, subHeight), capacity);
		children[3] = new GalaxyQuadTreeNode(sf::FloatRect(x + subWidth, y + subHeight, subWidth, subHeight), capacity);

		isLeaf = false;

		// Redistribute stars among children
		for (const auto &star : stars)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (children[i]->boundary.contains(star.getX(), star.getY()))
				{
					children[i]->insert(star);
					break;
				}
			}
		}
		stars.clear(); // Clear stars from this node after redistribution
	}
};
