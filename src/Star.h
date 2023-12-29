// Star.h
#ifndef STAR_H
#define STAR_H

#include <SFML/Graphics/Color.hpp>
#include <string>
class Star
{
public:
	Star(uint32_t ID, int x, int y, const std::string &name, const sf::Color &colour);
	uint32_t getID() const;
	int getX() const;
	int getY() const;
	std::string getName() const;
	sf::Color getColour() const;
	bool getIsExplored() const;
	void setIsExplored(bool newIsExploredValue);

private:
	uint32_t ID;
	int x;
	int y;
	std::string name;
	sf::Color colour;
	bool isExplored;
};

#endif // STAR_H