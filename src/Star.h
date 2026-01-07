// Star.h
#ifndef STAR_H
#define STAR_H

#include <cstdint>
#include <atomic>
#include <SFML/Graphics/Color.hpp>
#include <string>
class Star
{
public:
	Star(uint32_t ID, int x, int y, const std::string &name, const sf::Color &colour);
	// copy/move support needed because of the atomic member
	Star(const Star &other);
	Star &operator=(const Star &other);
	Star(Star &&other) noexcept;
	Star &operator=(Star &&other) noexcept;
	uint32_t getID() const;
	int getX() const;
	int getY() const;
	std::string getName() const;
	sf::Color getColour() const;
	bool getIsExplored() const;
	void setIsExplored(bool newIsExploredValue);
	// Atomically attempt to mark the star explored. Returns true if this call changed
	// the state from not-explored -> explored.
	bool tryMarkExplored();

private:
	uint32_t ID;
	int x;
	int y;
	std::string name;
	sf::Color colour;
	std::atomic<bool> isExplored;
};

#endif // STAR_H