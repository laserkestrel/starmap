// GalaxyQuadTree.cpp
#include "GalaxyQuadTree.h"

// Implement the constructor
GalaxyQuadTree::GalaxyQuadTree(const sf::FloatRect &boundary_, int capacity)
    : root(nullptr), capacity(capacity), boundary(boundary_), starVec(nullptr)
{
    // root will be initialized when starVec is set so nodes have access to star data
    root = new GalaxyQuadTreeNode(this->boundary, this->capacity, this->starVec);
}

// Implement the insert method
// Set pointer to canonical star vector used by nodes
void GalaxyQuadTree::setStarVector(const std::vector<Star> *sv)
{
    starVec = sv;
    if (root)
    {
        // reassign starVec to root and any existing children would have been assigned during splits
        root->starVec = starVec;
    }
}

// Insert a star by index into the quadtree
void GalaxyQuadTree::insert(size_t starIndex)
{
    if (root == nullptr)
    {
        root = new GalaxyQuadTreeNode(boundary, capacity, starVec);
    }
    root->insert(starIndex);
}

void GalaxyQuadTree::debugPrint() const
{
    if (root)
    {
        root->debugPrint();
    }
}