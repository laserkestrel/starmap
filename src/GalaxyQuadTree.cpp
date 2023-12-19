// GalaxyQuadTree.cpp
#include "GalaxyQuadTree.h"

// Implement the constructor
GalaxyQuadTree::GalaxyQuadTree(const sf::FloatRect &boundary, int capacity)
    : root(nullptr), capacity(capacity)
{
    // Initialize root and other necessary components
    root = new GalaxyQuadTreeNode(boundary, capacity); // Initialize root with the provided boundary and capacity
}

// Implement the insert method
void GalaxyQuadTree::insert(const Star &star)
{
    // If the root node is not initialized, create it and assign the boundary and capacity
    if (root == nullptr)
    {
        root = new GalaxyQuadTreeNode(boundary, capacity);
    }
    // Insert the star into the root node or recursively call an insertion method on the root node
    root->insert(star);
}

void GalaxyQuadTree::debugPrint() const
{
    if (root)
    {
        root->debugPrint();
    }
}