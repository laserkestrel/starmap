// Star.cpp
#include "Star.h"

Star::Star(uint32_t ID, int x, int y, const std::string &name, const sf::Color &colour) : ID(ID),
																						  x(x),
																						  y(y),
																						  name(name),
																						  colour(colour),
																						  // metals(metals),
																						  // polymers(polymers),
																						  // fuel(fuel),
																						  isExplored(false)
{
	// why does this have to be here?
}

// Copy constructor
Star::Star(const Star &other) : ID(other.ID), x(other.x), y(other.y), name(other.name), colour(other.colour), isExplored(other.isExplored.load())
{
}

// Copy assignment
Star &Star::operator=(const Star &other)
{
	if (this != &other)
	{
		ID = other.ID;
		x = other.x;
		y = other.y;
		name = other.name;
		colour = other.colour;
		isExplored.store(other.isExplored.load());
	}
	return *this;
}

// Move constructor
Star::Star(Star &&other) noexcept : ID(other.ID), x(other.x), y(other.y), name(std::move(other.name)), colour(other.colour), isExplored(other.isExplored.load())
{
}

// Move assignment
Star &Star::operator=(Star &&other) noexcept
{
	if (this != &other)
	{
		ID = other.ID;
		x = other.x;
		y = other.y;
		name = std::move(other.name);
		colour = other.colour;
		isExplored.store(other.isExplored.load());
	}
	return *this;
}

uint32_t Star::getID() const
{
	return ID;
}

int Star::getX() const
{
	return x;
}

int Star::getY() const
{
	return y;
}

std::string Star::getName() const
{
	return name;
}

sf::Color Star::getColour() const
{
	return colour;
}

/*
double Star::getMetals() const
{
	return metals;
}

double Star::getPolymers() const
{
	return polymers;
}

double Star::getFuel() const
{
	return fuel;
}
*/

bool Star::getIsExplored() const
{
	return isExplored.load();
}

// Setters
/*
void Star::setMetals(double newMetalsValue)
{
	metals = newMetalsValue;
}

void Star::setPolymers(double newPolymersValue)
{
	polymers = newPolymersValue;
}

void Star::setFuel(double newFuelValue)
{
	fuel = newFuelValue;
}
*/

void Star::setIsExplored(bool newIsExploredValue)
{
	isExplored.store(newIsExploredValue);
}

bool Star::tryMarkExplored()
{
	bool expected = false;
	// If it was false, set to true and return true; otherwise return false.
	return isExplored.compare_exchange_strong(expected, true);
}