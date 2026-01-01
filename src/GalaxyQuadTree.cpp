// GalaxyQuadTree.cpp
#include "GalaxyQuadTree.h"

// Implement the constructor
GalaxyQuadTree::GalaxyQuadTree(const sf::FloatRect &boundary_, int capacity)
    : root(nullptr), capacity(capacity), boundary(boundary_)
{
    // Initialize root with the provided boundary and capacity
    root = new GalaxyQuadTreeNode(this->boundary, this->capacity);
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