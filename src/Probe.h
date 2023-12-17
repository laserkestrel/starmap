// Probe.h
#ifndef PROBE_H
#define PROBE_H

#include "Star.h"
#include <SFML/Graphics.hpp>	   // Include SFML for colors
#include <SFML/System/Vector2.hpp> // Include SFML for Vector2f
#include <string>
#include <vector>

enum class ProbeMode
{
	Travel,
	Replicate,
	Seek,
	Shutdown
};
struct VisitedStarSystem
{
	std::string systemName;
	sf::Vector2f coordinates; // Coordinates of the star system
	bool visitedByProbe;	  // Indicator to differentiate direct visitation by probe (true) or parent (false)
};

class Probe
{
public:
	Probe(const std::string& probeName, float initialX, float initialY, float speed, const std::vector<Star>& galaxyVector); // Probes require access to the same shared galaxyVector object to update resources there.

	// Destructor
	~Probe();

	// Setters
	void setCoordinates(float x, float y);
	void setSpeed(float speed);
	void setMode(ProbeMode mode);
	void setTargetCoordinates(float newX, float newY);
	void setNewBorn(bool status);
	void addVisitedStarSystem(const std::string& systemName, const sf::Vector2f& coordinates, bool visitedByProbe);
	void setRandomTrailColor();
	void setTargetStar(std::string& setTargetStar);

	// Getters
	std::string getProbeName() const;
	float getX() const;
	float getY() const;
	float getSpeed() const;
	std::string getTargetStar() const;
	ProbeMode getMode() const;
	bool isNewBorn() const;
	float getTotalDistanceTraveled() const;
	int getReplicationCount() const;
	int getVisitedStarCount() const;
	const std::vector<VisitedStarSystem>& getVisitedStarSystems() const;
	sf::Color getTrailColor() const; // Declaration of getTrailColor method

	// Other methods
	void move(); // Example method representing movement logic

private:
	std::string probeName;
	float x;
	float y;
	float targetX;
	float targetY;
	std::string targetStar;
	float speed;
	ProbeMode mode;
	std::vector<VisitedStarSystem> visitedStarSystems; // a vector named visitedStarSystems that contains elements of type VisitedStarSystem
	std::vector<Star> galaxyVector;
	//std::vector<sf::Vector2f> probeTrail; // Member variable to store the trail of places - this is redunant, use the visitedStarSystems instead.
	bool newBorn;
	float totalDistanceTraveled;
	int replicationCount;
	int visitedStarCount;
	const Star* findNearestUnvisitedStar() const;
	const Star* findNearestUnvisitedStarByRadius() const;
	sf::Color trailColor; // Declaration of trailColor within the class
};

#endif // PROBE_H