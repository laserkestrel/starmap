//Star.h
#ifndef STAR_H
#define STAR_H

#include <SFML/Graphics/Color.hpp> // Include the SFML header for sf::Color
#include <string>
class Star
{
public:
	Star(int x, int y, const std::string& name, const sf::Color& colour, double metals, double polymers, double fuel);

	int getX() const;
	int getY() const;
	std::string getName() const;
	sf::Color getColour() const; // Change the return type to sf::Color
	double getMetals() const;
	double getPolymers() const;
	double getFuel() const;
	bool getIsExplored() const;
	void setMetals(double newMetalsValue);
	void setPolymers(double newPolymersValue);
	void setFuel(double newFuelValue);
	void setIsExplored(bool newIsExploredValue);

private:
	int x;
	int y;
	std::string name;
	sf::Color colour; // Change the type to sf::Color
	double metals;
	double polymers;
	double fuel;
	bool isExplored;
};

#endif // STAR_H