// Star.cpp
#include "Star.h"

Star::Star(uint32_t ID, float worldX, float worldY, float worldZ, const std::string &name,
		   const sf::Color &colour, float displayBrightness)
	: ID(ID), worldX(worldX), worldY(worldY), worldZ(worldZ), name(name), colour(colour),
	  displayBrightness(displayBrightness), isExplored(false)
{
}

// Copy constructor -- std::atomic is neither copyable nor movable, so the whole
// set below has to be written out by hand.
Star::Star(const Star &other)
	: ID(other.ID), worldX(other.worldX), worldY(other.worldY), worldZ(other.worldZ),
	  name(other.name), colour(other.colour), displayBrightness(other.displayBrightness),
	  isExplored(other.isExplored.load())
{
}

Star &Star::operator=(const Star &other)
{
	if (this != &other)
	{
		ID = other.ID;
		worldX = other.worldX;
		worldY = other.worldY;
		worldZ = other.worldZ;
		name = other.name;
		colour = other.colour;
		displayBrightness = other.displayBrightness;
		isExplored.store(other.isExplored.load());
	}
	return *this;
}

Star::Star(Star &&other) noexcept
	: ID(other.ID), worldX(other.worldX), worldY(other.worldY), worldZ(other.worldZ),
	  name(std::move(other.name)), colour(other.colour), displayBrightness(other.displayBrightness),
	  isExplored(other.isExplored.load())
{
}

Star &Star::operator=(Star &&other) noexcept
{
	if (this != &other)
	{
		ID = other.ID;
		worldX = other.worldX;
		worldY = other.worldY;
		worldZ = other.worldZ;
		name = std::move(other.name);
		colour = other.colour;
		displayBrightness = other.displayBrightness;
		isExplored.store(other.isExplored.load());
	}
	return *this;
}

uint32_t Star::getID() const { return ID; }
float Star::getWorldX() const { return worldX; }
float Star::getWorldY() const { return worldY; }
float Star::getWorldZ() const { return worldZ; }

// Returned by reference: initializeStarsTexture asks for this three times per
// star per rebuild, and copying the string each time was pure waste.
const std::string &Star::getName() const { return name; }

sf::Color Star::getColour() const { return colour; }
float Star::getDisplayBrightness() const { return displayBrightness; }
bool Star::getIsExplored() const { return isExplored.load(); }
void Star::setIsExplored(bool newIsExploredValue) { isExplored.store(newIsExploredValue); }

bool Star::tryMarkExplored()
{
	bool expected = false;
	// If it was false, set to true and return true; otherwise return false.
	return isExplored.compare_exchange_strong(expected, true);
}
