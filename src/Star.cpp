//Star.cpp
#include "Star.h"

Star::Star(int x, int y, const std::string& name, const sf::Color& colour,
	double metals, double polymers, double fuel) :
	x(x),
	y(y),
	name(name),
	colour(colour),
	metals(metals),
	polymers(polymers),
	fuel(fuel),
	isExplored(false)
{
	//why does this have to be here?
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

bool Star::getIsExplored() const
{
	return isExplored;
}

//Setters
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

void Star::setIsExplored(bool newIsExploredValue)
{
	isExplored = newIsExploredValue;
}