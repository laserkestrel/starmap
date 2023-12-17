// Probe.cpp
#include "Probe.h"
#include "Star.h"
#include <algorithm> // For std::find
#include <cmath>	 // For std::sqrt
#include <iostream>
#include <limits>
#include <random>

// Constructor (these are things that get set on a new instance)
Probe::Probe(const std::string& probeName, float initialX, float initialY, float speed, const std::vector<Star>& galaxyVector) :
	probeName(probeName),
	x(initialX),
	y(initialY),
	speed(speed),
	mode(ProbeMode::Seek),
	galaxyVector(galaxyVector),
	newBorn(true),
	totalDistanceTraveled(0.0f),
	replicationCount(0),
	visitedStarCount(0)
{
	// Additional setup if needed
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

void Probe::setTargetStar(std::string& targetStar)
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
	//std::cout << "setMode has been invoked\n";
}

void Probe::setTargetCoordinates(float newX, float newY)
{
	targetX = newX;
	targetY = newY;
}

void Probe::addVisitedStarSystem(const std::string& systemName, const sf::Vector2f& coordinates, bool visitedByProbe)
{
	VisitedStarSystem visitedSystem = { systemName, coordinates, visitedByProbe };
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

std::string Probe::getTargetStar() const
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
		// Implement travel behavior
		//std::cout << "Probe is traveling.\n";
		// Calculate the direction from current position to the target position
		float deltaX = targetX - x;
		float deltaY = targetY - y;
		float distanceToTarget = sqrt(deltaX * deltaX + deltaY * deltaY);

		// Calculate the movement step size based on speed
		float stepSize = speed; // Adjust this based on your needs

		if (distanceToTarget <= stepSize)
		{
			// If the distance to the target is smaller than or equal to the step size,
			// move directly to the target position
			setCoordinates(targetX, targetY);
			// we have arrived, implement some new behaviour

			// Update distance traveled
			totalDistanceTraveled += distanceToTarget;
			//update probe memory with newly arrived star, before finding next target.
			addVisitedStarSystem(this->getTargetStar(), sf::Vector2f(this->getX(), this->getY()), true);

			// Update the target star's isExplored value to true
			for (auto& star : galaxyVector)
			{
				if (star.getName() == this->getTargetStar())
				{
					// Found the matching star, update its isExplored value
					star.setIsExplored(true);
					break; // Exit the loop once the star is found and updated
				}
			}

			/* this is seek behaviour, dont do this in travel mode!
			const Star* nearestStar = findNearestUnvisitedStar();
			if (nearestStar)
			{
				std::cout << "Probe at (" << x << ", " << y << ") has nearest star: " << nearestStar->getName() << std::endl;
				//addVisitedStarSystem(nearestStar->getName(), sf::Vector2f(nearestStar->getX(), nearestStar->getY()), true);
			}
			else
			{
				std::cout << "No nearest unvisited star found for the probe at (" << x << ", " << y << ").\n";
				// Handle the case where there's no nearest unvisited star
			}*/
			if (this->isNewBorn())
			{
				this->setNewBorn(false);
				setMode(ProbeMode::Seek);
			}
			else
			{
				// Set mode to Seek (to have one probe just move around) or replicate to start spreading.(ONLY IF NOT NEWBORN)
				setMode(ProbeMode::Replicate);
			}
		}
		else
		{
			// Calculate the normalized direction vector
			float directionX = deltaX / distanceToTarget;
			float directionY = deltaY / distanceToTarget;

			// Move towards the target position by the step size in the direction of the target
			setCoordinates(x + directionX * stepSize, y + directionY * stepSize);
			//setSpeed(100); // dont need to set this here. gets auto set in travel.
		}
	}
	else if (mode == ProbeMode::Replicate)
	{
		// Implement replicate behavior
		//std::cout << "Probe would replicate now inside probe.cpp.\n";
		// This now does nothing, because replicate is checked in game.cpp
		replicationCount++;
		setMode(ProbeMode::Seek);
	}
	else if (mode == ProbeMode::Seek)
	{
		if (galaxyVector.empty())
		{
			std::cout << "No stars available for seeking.\n";
			setMode(ProbeMode::Shutdown);
			return;
		}

		if (this->isNewBorn() && !visitedStarSystems.empty())
		{
			#if defined(_DEBUG)
			std::cout << "Probe is newborn so will seek new direction from parent.\n";
			#endif
			// Define an offset range from the parent's position
			std::uniform_real_distribution<float> disAngle(0.0f, 2.0f * 3.14159f); // Angle range for full circle
			std::uniform_real_distribution<float> disDistance(150.0f, 200.0f);	   // Distance range from 50 to 100 pixels
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
			const Star* nearestStar = findNearestUnvisitedStarByRadius();

			if (nearestStar)
			{
				#if defined(_DEBUG)
				std::cout << "Probe [" << this->probeName << "] is seeking the nearest star: " << nearestStar->getName() << ".\n";
				std::cout << "Nearest Star Coordinates: (" << nearestStar->getX() << ", " << nearestStar->getY() << ")" << std::endl;
				#endif
				this->setTargetCoordinates(nearestStar->getX(), nearestStar->getY());
				std::string newTarget = (nearestStar->getName()); //NOTE:have to create intermediate string for star name to then pass into setTargetStar. Complains if done directly.
				this->setTargetStar(newTarget);
				setMode(ProbeMode::Travel);
				this->setSpeed(10);
			}
			else
			{
				#if defined(_DEBUG)
				std::cout << "All stars have been visited by probe and it has no new stars to seek.\n";
				#endif
				setMode(ProbeMode::Shutdown);
			}
		}
	}
	else if (mode == ProbeMode::Shutdown)
	{
		// remove probe from vector for logic processing or set some requiresLogic property to false?
		#if defined(_DEBUG)
		std::cout << "Probe has shutdown.\n";
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

int Probe::getVisitedStarCount() const
{
	return visitedStarCount;
}

const std::vector<VisitedStarSystem>& Probe::getVisitedStarSystems() const
{
	return visitedStarSystems;
}

sf::Color Probe::getTrailColor() const
{
	return trailColor; // Return the trailColor
}

// Implementation of findNearestUnvisitedStar method
const Star* Probe::findNearestUnvisitedStar() const
{
	const Star* nearestStar = nullptr;
	float minDistance = std::numeric_limits<float>::max();

	for (const Star& star : galaxyVector)
	{
		if (std::find_if(visitedStarSystems.begin(), visitedStarSystems.end(), [&star](const VisitedStarSystem& visitedSystem) {
				return visitedSystem.systemName == star.getName();
			}) == visitedStarSystems.end()
			&& !(x == star.getX() && y == star.getY()))
		{
			float distance = std::sqrt(std::pow(x - star.getX(), 2) + std::pow(y - star.getY(), 2));

			if (distance < minDistance)
			{
				minDistance = distance;
				nearestStar = &star;
			}
		}
	}
	std::cout << "findNearestUnvisitedStar has just been called.\n";
	return nearestStar;
}

const Star* Probe::findNearestUnvisitedStarByRadius() const
{
	float radius = 250.0f; // Set a default radius value

	const Star* nearestStar = nullptr;
	float minDistance = std::numeric_limits<float>::max();

	for (const Star& star : galaxyVector)
	{
		// Calculate distance between probe and the star
		float distance = std::sqrt(std::pow(x - star.getX(), 2) + std::pow(y - star.getY(), 2));

		// Check if the star is unvisited and within the specified radius
		if (distance <= radius && std::find_if(visitedStarSystems.begin(), visitedStarSystems.end(), [&star](const VisitedStarSystem& visitedSystem) {
				return visitedSystem.systemName == star.getName();
			}) == visitedStarSystems.end()
			&& !(x == star.getX() && y == star.getY()))
		{
			if (distance < minDistance)
			{
				minDistance = distance;
				nearestStar = &star;
			}
		}
	}
	//std::cout << "findNearestUnvisitedStarByRadius has just been called.\n";
	return nearestStar;
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