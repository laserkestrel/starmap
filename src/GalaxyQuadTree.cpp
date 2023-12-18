#include "GalaxyQuadTree.h"

// Implement the constructor
GalaxyQuadTree::GalaxyQuadTree(const sf::FloatRect &boundary, int capacity)
    : root(nullptr), capacity(capacity)
{
    // Initialize root and other necessary components
}

// Implement the insert method
void GalaxyQuadTree::insert(const Star &star)
{
    // Logic to insert the star into the quadtree
    // Traverse the tree, split nodes if needed, and add the star to an appropriate node
    if (root == nullptr)
    {
        // root = new GalaxyQuadTreeNode(boundary, capacity);
    }

    // insertRecursive(root, star);
}

// Implement the query method
std::vector<Star> GalaxyQuadTree::query(const sf::Vector2f &point, float radius)
{
    std::vector<Star> foundStars;
    // Logic to query stars within a radius around a point
    // Traverse the tree, check intersections, and collect stars within the radius
    return foundStars;
}

// Implement other private helper methods, node splitting, etc.
