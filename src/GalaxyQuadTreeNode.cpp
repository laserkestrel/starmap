// GalaxyQuadTreeNode.cpp
#include "GalaxyQuadTreeNode.h"
#include <iostream>
#include <string>

GalaxyQuadTreeNode::GalaxyQuadTreeNode(const sf::FloatRect &nodeBoundary, int nodeCapacity,
									   const std::vector<Star> *starVecPtr, int nodeDepth)
	: boundary(nodeBoundary), starVec(starVecPtr), capacity(nodeCapacity), depth(nodeDepth)
{
}

const GalaxyQuadTreeNode *GalaxyQuadTreeNode::getChild(int index) const
{
	if (index >= 0 && index < 4)
	{
		return children[index].get();
	}
	return nullptr;
}

void GalaxyQuadTreeNode::setStarVector(const std::vector<Star> *sv)
{
	starVec = sv;
	for (auto &child : children)
	{
		if (child)
		{
			child->setStarVector(sv);
		}
	}
}

bool GalaxyQuadTreeNode::insert(size_t starIndex)
{
	if (starVec == nullptr)
		return false;

	const Star &star = (*starVec)[starIndex];

	if (!boundary.contains(star.getWorldX(), star.getWorldY()))
	{
		return false;
	}

	// At the depth limit the leaf simply grows past its capacity. Dropping the
	// star instead would be worse: a lost star is invisible to every probe.
	if (isLeaf && (starIndices.size() < static_cast<size_t>(capacity) || depth >= MAX_DEPTH))
	{
		starIndices.push_back(starIndex);
		return true;
	}

	if (isLeaf)
	{
		split();
	}

	for (auto &child : children)
	{
		if (child->insert(starIndex))
		{
			return true;
		}
	}

	// Inside this node but rejected by all four children -- only reachable
	// through floating point edge cases. Keep it here rather than lose it.
	starIndices.push_back(starIndex);
	return true;
}

void GalaxyQuadTreeNode::queryRange(const sf::FloatRect &area, std::vector<size_t> &out) const
{
	if (starVec == nullptr || !area.intersects(boundary))
	{
		return;
	}

	for (const auto &idx : starIndices)
	{
		const Star &star = (*starVec)[idx];
		if (area.contains(star.getWorldX(), star.getWorldY()))
		{
			out.push_back(idx);
		}
	}

	for (const auto &child : children)
	{
		if (child)
		{
			child->queryRange(area, out);
		}
	}
}

void GalaxyQuadTreeNode::split()
{
	const float subWidth = boundary.width / 2.0f;
	const float subHeight = boundary.height / 2.0f;
	const float x = boundary.left;
	const float y = boundary.top;

	children[0] = std::make_unique<GalaxyQuadTreeNode>(sf::FloatRect(x + subWidth, y, subWidth, subHeight), capacity, starVec, depth + 1);
	children[1] = std::make_unique<GalaxyQuadTreeNode>(sf::FloatRect(x, y, subWidth, subHeight), capacity, starVec, depth + 1);
	children[2] = std::make_unique<GalaxyQuadTreeNode>(sf::FloatRect(x, y + subHeight, subWidth, subHeight), capacity, starVec, depth + 1);
	children[3] = std::make_unique<GalaxyQuadTreeNode>(sf::FloatRect(x + subWidth, y + subHeight, subWidth, subHeight), capacity, starVec, depth + 1);

	isLeaf = false;

	std::vector<size_t> toRedistribute;
	toRedistribute.swap(starIndices);
	for (const auto &idx : toRedistribute)
	{
		bool placed = false;
		for (auto &child : children)
		{
			if (child->insert(idx))
			{
				placed = true;
				break;
			}
		}
		if (!placed)
		{
			starIndices.push_back(idx); // keep it at this level rather than lose it
		}
	}
}

void GalaxyQuadTreeNode::debugPrint(int indentDepth) const
{
	const std::string indent(indentDepth * 2, ' ');

	std::cout << indent << "Node boundary (pc): " << boundary.left << ", " << boundary.top << ", "
			  << boundary.width << ", " << boundary.height << " [" << starIndices.size() << " stars]" << std::endl;

	if (!isLeaf)
	{
		for (const auto &child : children)
		{
			if (child)
			{
				child->debugPrint(indentDepth + 1);
			}
		}
	}
}
