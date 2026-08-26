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
	Star(uint32_t ID, float worldX, float worldY, float worldZ, const std::string &name,
		 const sf::Color &colour, float displayBrightness);
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
	// 0..1, the magnitude-derived brightness already folded into the colour.
	// The renderer uses it to size the star, since brighter stars read as larger.
	float getDisplayBrightness() const;
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
	float displayBrightness = 1.0f;
	std::atomic<bool> isExplored;
};

#endif // STAR_H
