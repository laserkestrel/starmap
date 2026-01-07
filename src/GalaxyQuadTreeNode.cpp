// GalaxyQuadTreeNode.cpp
#include "GalaxyQuadTreeNode.h"
#include <iostream>
#include <string>

GalaxyQuadTreeNode::GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity, const std::vector<Star> *starVecPtr)
	: boundary(nodeBoundary), starIndices(), starVec(starVecPtr), isLeaf(true), capacity(nodeCapacity)
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

bool GalaxyQuadTreeNode::insert(size_t starIndex)
{
	if (starVec == nullptr)
		return false;

	const Star &star = (*starVec)[starIndex];

	if (!boundary.contains(star.getX(), star.getY()))
	{
		return false;
	}

	if (isLeaf && starIndices.size() < static_cast<size_t>(capacity))
	{
		starIndices.push_back(starIndex);
		return true;
	}

	if (isLeaf)
	{
		split();
	}

	for (int i = 0; i < 4; ++i)
	{
		if (children[i]->insert(starIndex))
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

	children[0] = new GalaxyQuadTreeNode(sf::FloatRect(x + subWidth, y, subWidth, subHeight), capacity, starVec);
	children[1] = new GalaxyQuadTreeNode(sf::FloatRect(x, y, subWidth, subHeight), capacity, starVec);
	children[2] = new GalaxyQuadTreeNode(sf::FloatRect(x, y + subHeight, subWidth, subHeight), capacity, starVec);
	children[3] = new GalaxyQuadTreeNode(sf::FloatRect(x + subWidth, y + subHeight, subWidth, subHeight), capacity, starVec);

	isLeaf = false;

	for (const auto &idx : starIndices)
	{
		const Star &star = (*starVec)[idx];
		for (int i = 0; i < 4; ++i)
		{
			if (children[i]->boundary.contains(star.getX(), star.getY()))
			{
				children[i]->insert(idx);
				break;
			}
		}
	}
	starIndices.clear();
}

void GalaxyQuadTreeNode::debugPrint(int depth) const
{
	std::string indent(depth * 2, ' '); // Create an indent based on the depth

	std::cout << indent << "Node Boundary: " << boundary.left << ", " << boundary.top << ", "
			  << boundary.width << ", " << boundary.height << std::endl;

	if (isLeaf)
	{
		for (const auto &idx : starIndices)
		{
			const Star &star = (*starVec)[idx];
			std::cout << indent << "  Star: ";

			if (!star.getName().empty())
			{
				std::cout << star.getName();
			}
			else
			{
				std::cout << "Unnamed";
			}

			std::cout << " (" << star.getX() << ", " << star.getY() << ")" << std::endl;
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