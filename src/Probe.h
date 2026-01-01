// Probe.h
#ifndef PROBE_H
#define PROBE_H

#include "Star.h"
#include <SFML/Graphics.hpp>	   // Include SFML for colors
#include <SFML/System/Vector2.hpp> // Include SFML for Vector2f
#include <string>
#include <vector>
#include "GalaxyQuadTree.h"
#include "LoadConfig.h"

enum class ProbeMode
{
	Travel,
	Replicate,
	Seek,
	Shutdown
};

struct VisitedStarSystem // Probes own private memory of visited systems.
{
	// std::string systemName;
	uint32_t starID;
	sf::Vector2f coordinates; // Coordinates of the star system
	bool visitedByProbe;	  // Indicator to differentiate direct visitation by probe (true) or parent (false)
};

// The constructor for any class .h file is defined in the class under the "public" section. In C++, the constructor is a special member function with the same name as the class, and it is used for initializing the object's state when an instance of the class is created.
class Probe
{
public:
	Probe(const std::string &probeName, float initialX, float initialY, float speed, GalaxyQuadTree &quadTree); // Probes require access to the same shared galaxyVector object to update resources there.
	// Destructor
	~Probe();

	std::string visitedStarSystemsToString() const;

	// Setters
	void setCoordinates(float x, float y);
	void setSpeed(float speed);
	void setMode(ProbeMode mode);
	void setTargetCoordinates(float newX, float newY);
	void setNewBorn(bool status);
	void addVisitedStarSystem(const uint32_t &starID, const sf::Vector2f &coordinates, bool visitedByProbe);
	void setRandomTrailColor();
	void setBlackTrailColor();
	void setTargetStar(uint32_t &setTargetStar);

	// Getters
	std::string getProbeName() const;
	float getX() const;
	float getY() const;
	float getSpeed() const;
	uint32_t getTargetStar() const;
	ProbeMode getMode() const;
	bool isNewBorn() const;
	float getTotalDistanceTraveled() const;
	int getReplicationCount() const;
	// int getVisitedStarCount() const;
	const std::vector<VisitedStarSystem> &getVisitedStarSystems() const;
	sf::Color getTrailColor() const; // Declaration of getTrailColor method

	// Other methods
	void move(); // Example method representing movement logic
	const Star *findNearestUnvisitedStarInQuadTree(const GalaxyQuadTreeNode *node, float searchRadius) const;
	const GalaxyQuadTreeNode *getCurrentQuadTreeNode() const
	{
		return currentQuadTreeNode;
	}

private:
	std::string probeName;
	float x;
	float y;
	float targetX;
	float targetY;
	uint32_t targetStar;
	float speed;
	ProbeMode mode;
	std::vector<VisitedStarSystem> visitedStarSystems; // a vector named visitedStarSystems that contains elements of type VisitedStarSystem
	GalaxyQuadTree &quadTree;
	const GalaxyQuadTreeNode *currentQuadTreeNode;
	bool newBorn;
	float totalDistanceTraveled;
	int replicationCount;
	sf::Color trailColor; // Declaration of trailColor within the class
	LoadConfig *myConfigInstance;
};

#endif // PROBE_H