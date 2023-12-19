// GalaxyQuadTreeNode.cpp
#include "GalaxyQuadTreeNode.h"
#include <iostream>
#include <string>

GalaxyQuadTreeNode::GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity)
	: boundary(nodeBoundary), isLeaf(true), capacity(nodeCapacity)
{
	for (int i = 0; i < 4; ++i)
	{
		children[i] = nullptr;
	}
}

GalaxyQuadTreeNode *GalaxyQuadTreeNode::getChild(int index) const
{
	if (index >= 0 && index < 4)
	{
		return children[index];
	}
	return nullptr;
}

bool GalaxyQuadTreeNode::insert(const Star &star)
{
	if (!boundary.contains(star.getX(), star.getY()))
	{
		return false;
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

void GalaxyQuadTreeNode::split()
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
	stars.clear();
}

void GalaxyQuadTreeNode::debugPrint(int depth) const
{
	std::string indent(depth * 2, ' '); // Create an indent based on the depth

	std::cout << indent << "Node Boundary: " << boundary.left << ", " << boundary.top << ", "
			  << boundary.width << ", " << boundary.height << std::endl;

	if (isLeaf)
	{
		for (const auto &star : stars)
		{
			std::cout << indent << "  Star: " << star.getName() << " (" << star.getX() << ", " << star.getY() << ")" << std::endl;
		}
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			if (children[i])
			{
				children[i]->debugPrint(depth + 1);
			}
		}
	}
}