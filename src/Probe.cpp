// Probe.cpp
#include "Probe.h"
#include "Star.h"
#include <algorithm> // For std::find
#include <cmath>	 // For std::sqrt
#include <iostream>
#include <limits>
#include <random>
#include "LoadConfig.h"

// Constructor (these are things that get set on a new instance)
// Probe::Probe(const std::string &probeName, float initialX, float initialY, float speed, const std::vector<Star> &galaxyVector, GalaxyQuadTree &quadTree)
Probe::Probe(const std::string &probeName, float initialX, float initialY, float speed, GalaxyQuadTree &quadTree) : probeName(probeName),
																													x(initialX),
																													y(initialY),
																													speed(speed),
																													mode(ProbeMode::Seek),
																													quadTree(quadTree),
																													newBorn(true),
																													totalDistanceTraveled(0.0f),
																													replicationCount(0),
																													myConfigInstance(&LoadConfig::getInstance())

// visitedStarCount(0)
{
	// Additional setup if needed
}

std::string Probe::visitedStarSystemsToString() const
{
	std::string result;

	for (const auto &visitedSystem : visitedStarSystems)
	{
		result += "Star ID: " + std::to_string(visitedSystem.starID) + result += " Coordinates: (" + std::to_string(visitedSystem.coordinates.x) + ", " + std::to_string(visitedSystem.coordinates.y) + ")\n";
		// result += "Visited by Probe: " + (visitedSystem.visitedByProbe ? "Yes" : "No") + "\n\n";
	}

	return result;
}

// Destructor
Probe::~Probe()
{
	// Cleanup if needed
}

// Setters
void Probe::setCoordinates(float x, float y)
{
	this->x = x;
	this->y = y;
}

void Probe::setTargetStar(uint32_t &targetStar)
{
	this->targetStar = targetStar;
}

void Probe::setSpeed(float speed)
{
	this->speed = speed;
}

void Probe::setMode(ProbeMode mode)
{
	this->mode = mode;
	// std::cout << "setMode has been invoked\n";
}

void Probe::setTargetCoordinates(float newX, float newY)
{
	targetX = newX;
	targetY = newY;
}

void Probe::addVisitedStarSystem(const uint32_t &starID, const sf::Vector2f &coordinates, bool visitedByProbe)
{
	VisitedStarSystem visitedSystem = {starID, coordinates, visitedByProbe};
	visitedStarSystems.push_back(visitedSystem);
}

// Getters
std::string Probe::getProbeName() const
{
	return probeName;
}

float Probe::getX() const
{
	return x;
}

float Probe::getY() const
{
	return y;
}

float Probe::getSpeed() const
{
	return speed;
}

uint32_t Probe::getTargetStar() const
{
	return targetStar;
}

ProbeMode Probe::getMode() const
{
	return mode;
}

void Probe::move()
{
	if (mode == ProbeMode::Travel)
	{
		// Calculate the direction from current position to the target position
		float deltaX = targetX - x;
		float deltaY = targetY - y;
		float distanceToTarget = sqrt(deltaX * deltaX + deltaY * deltaY);

		// Calculate the movement step size based on speed
		float stepSize = speed; // Adjust this based on your needs

		if (distanceToTarget <= stepSize)
		{
			// If the distance to the target is smaller than or equal to the step size, move directly to the target position
			setCoordinates(targetX, targetY);
			// Probe has arrived!

			// Update distance traveled
			totalDistanceTraveled += distanceToTarget;
			// update probe memory with newly arrived star, before finding next target.
			addVisitedStarSystem(this->getTargetStar(), sf::Vector2f(this->getX(), this->getY()), true);

			// TODO - need to set this probes current quadtree location so that we can use it as a search parameter from game class, so we can establish next valid target and stop child probe going there. phew.
			// Determine the current quadtree node based on the probe's position
			// ...

			// Update currentQuadTreeNode with the new node
			// currentQuadTreeNode = // ... the new node

			if (this->isNewBorn())
			{
				this->setNewBorn(false);
				setMode(ProbeMode::Seek);
			}
			else
			{
				// if (this->getReplicationCount() >= config.getprobeIndividualReplicationLimit()){
				// int globalSetting = myConfigInstance.getprobeIndividualReplicationLimit();
				// int globalSetting = LoadConfig::getInstance().getprobeIndividualReplicationLimit();

				if (this->getReplicationCount() >= myConfigInstance->getprobeIndividualReplicationLimit())
				{
					// do what we like here, keep seeking if cant replicate.
					// setMode(ProbeMode::Seek);
					setMode(ProbeMode::Shutdown);
				}
				else
				{
					// Set mode to Seek (to have one probe just move around) or replicate to start spreading.(ONLY IF NOT NEWBORN)
					setMode(ProbeMode::Replicate);
				}
			}
		}
		else
		{
			// Calculate the normalized direction vector
			float directionX = deltaX / distanceToTarget;
			float directionY = deltaY / distanceToTarget;

			// Move towards the target position by the step size in the direction of the target
			setCoordinates(x + directionX * stepSize, y + directionY * stepSize);
		}
	}
	else if (mode == ProbeMode::Replicate)
	{
		// This now does nothing, because replicate is checked in game.cpp
		replicationCount++;
		setMode(ProbeMode::Seek);
	}
	else if (mode == ProbeMode::Seek)
	{
		if (this->isNewBorn() && !visitedStarSystems.empty())
		{
			// Probe is newborn
			// Define an offset range from the parent's position
			std::uniform_real_distribution<float> disAngle(0.0f, 2.0f * 3.14159f); // Angle range for full circle
			std::uniform_real_distribution<float> disDistance(30.0f, 60.0f);	   // Distance range from 50 to 100 pixels
			std::random_device rd;
			std::mt19937 gen(rd());
			float randomAngle = disAngle(gen);		 // Random angle in radians
			float randomDistance = disDistance(gen); // Random distance

			// Calculate new coordinates based on random angle and distance from the parent's position
			float offsetX = randomDistance * std::cos(randomAngle);
			float offsetY = randomDistance * std::sin(randomAngle);

			// Update target coordinates within a range from the parent's position
			setTargetCoordinates(this->getX() + offsetX, this->getY() + offsetY);

			setMode(ProbeMode::Travel);
			setSpeed(10);
			return;
		}
		else
		{
			// const Star *nearestStar = findNearestUnvisitedStarByRadius();
			// setup a pointer (called nearestStar) to a star object returned by the finding method.
			const Star *nearestStar = findNearestUnvisitedStarInQuadTree(quadTree.getRootNode(), myConfigInstance->getProbeSearchRadiusPixels());

			if (nearestStar)
			{
				this->setTargetCoordinates(nearestStar->getX(), nearestStar->getY());
				uint32_t newTarget = (nearestStar->getID()); // NOTE:have to create intermediate variable for star ID to then pass into setTargetStar. Complains if done directly.
				this->setTargetStar(newTarget);
				setMode(ProbeMode::Travel);
				this->setSpeed(10);
			}
			else
			{
#if defined(_DEBUG)
				// std::cout << "All stars have been visited by probe and it has no new stars to seek.\n";// TOO VERBOSE
#endif
				setMode(ProbeMode::Shutdown);
			}
		}
	}
	else if (mode == ProbeMode::Shutdown)
	{
// remove probe from vector for logic processing or set some requiresLogic property to false?
#if defined(_DEBUG)
		// std::cout << "Probe has shutdown.\n"; // overly verbose
#endif
	}
	else
	{
		std::cout << "Unknown mode.\n";
	}
}

// Implementation of setNewBorn method
void Probe::setNewBorn(bool status)
{
	newBorn = status;
}

// Implementation of isNewBorn method
bool Probe::isNewBorn() const
{
	return newBorn;
}

float Probe::getTotalDistanceTraveled() const
{
	return totalDistanceTraveled;
}

int Probe::getReplicationCount() const
{
	return replicationCount;
}

/*int Probe::getVisitedStarCount() const
{
	return visitedStarCount;
}
*/

const std::vector<VisitedStarSystem> &Probe::getVisitedStarSystems() const
{
	return visitedStarSystems;
}

sf::Color Probe::getTrailColor() const
{
	return trailColor; // Return the trailColor
}

void Probe::setBlackTrailColor()
{
	// Generate RGB values
	sf::Uint8 red = 0;
	sf::Uint8 green = 0;
	sf::Uint8 blue = 0;
	// Set the trail color
	trailColor = sf::Color(red, green, blue);
}

void Probe::setRandomTrailColor()
{
	const int minBrightness = 100; // Minimum brightness value to ensure the color is visible

	// Generate random RGB values
	sf::Uint8 red = static_cast<sf::Uint8>(std::rand() % 256);
	sf::Uint8 green = static_cast<sf::Uint8>(std::rand() % 256);
	sf::Uint8 blue = static_cast<sf::Uint8>(std::rand() % 256);

	// Calculate brightness of the color
	float brightness = (0.299f * red + 0.587f * green + 0.114f * blue);

	// Check if brightness is too low, adjust if necessary
	if (brightness < minBrightness)
	{
		float ratio = minBrightness / brightness;
		red = static_cast<sf::Uint8>(std::min(255, static_cast<int>(red * ratio)));
		green = static_cast<sf::Uint8>(std::min(255, static_cast<int>(green * ratio)));
		blue = static_cast<sf::Uint8>(std::min(255, static_cast<int>(blue * ratio)));
	}

	// Set the trail color
	trailColor = sf::Color(red, green, blue);
}

const Star *Probe::findNearestUnvisitedStarInQuadTree(const GalaxyQuadTreeNode *node, float searchRadius) const
{
	if (node == nullptr)
	{
		return nullptr;
	}

	const Star *nearestStar = nullptr;
	float minDistance = std::numeric_limits<float>::max();

	sf::FloatRect searchArea(x - searchRadius, y - searchRadius, searchRadius * 2, searchRadius * 2);
	if (node->boundary.intersects(searchArea))
	{
		for (const auto &star : node->stars)
		{
			if (!star.getIsExplored() && std::find_if(visitedStarSystems.begin(), visitedStarSystems.end(), [&star](const VisitedStarSystem &visitedSystem)
													  { return visitedSystem.starID == star.getID(); }) == visitedStarSystems.end())
			{

				float distance = std::sqrt(std::pow(star.getX() - x, 2) + std::pow(star.getY() - y, 2));
				if (distance <= searchRadius && distance < minDistance)
				{
					minDistance = distance;
					nearestStar = &star;
				}
			}
		}

		if (!node->isLeaf)
		{
			for (int i = 0; i < 4; ++i)
			{
				const Star *childNearestStar = findNearestUnvisitedStarInQuadTree(node->getChild(i), searchRadius);
				if (childNearestStar)
				{
					float childDistance = std::sqrt(std::pow(childNearestStar->getX() - x, 2) + std::pow(childNearestStar->getY() - y, 2));
					if (childDistance < minDistance)
					{
						minDistance = childDistance;
						nearestStar = childNearestStar;
					}
				}
			}
		}
	}

	return nearestStar;
}