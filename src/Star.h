// Star.h
#ifndef STAR_H
#define STAR_H

#include <cstdint>
#include <atomic>
#include <SFML/Graphics/Color.hpp>
#include <string>

// A star at its true position in world space -- parsecs, in the catalogue's
// equatorial Cartesian frame, with Sol at the origin. Nothing here knows about
// pixels, windows or the current view; that is Projection's job.
class Star
{
public:
	Star(uint32_t ID, float worldX, float worldY, float worldZ, const std::string &name, const sf::Color &colour);
	// copy/move support needed because of the atomic member
	Star(const Star &other);
	Star &operator=(const Star &other);
	Star(Star &&other) noexcept;
	Star &operator=(Star &&other) noexcept;

	uint32_t getID() const;
	float getWorldX() const;
	float getWorldY() const;
	float getWorldZ() const;
	const std::string &getName() const;
	sf::Color getColour() const;
	bool getIsExplored() const;
	void setIsExplored(bool newIsExploredValue);
	// Atomically attempt to mark the star explored. Returns true if this call changed
	// the state from not-explored -> explored.
	bool tryMarkExplored();

private:
	uint32_t ID = 0;
	float worldX = 0.0f;
	float worldY = 0.0f;
	float worldZ = 0.0f;
	std::string name;
	sf::Color colour;
	std::atomic<bool> isExplored;
};

#endif // STAR_H
