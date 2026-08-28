// Star.h
#ifndef STAR_H
#define STAR_H

#include <cstdint>
#include <atomic>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include "Resources.h"

// A star at its true position in world space -- parsecs, in the catalogue's
// equatorial Cartesian frame, with Sol at the origin. Nothing here knows about
// pixels, windows or the current view; that is Projection's job.
//
// A system also holds a finite stock of three resources. Twelve extra bytes per
// star sounds like the expensive decision, and it is worth being clear that it is
// not: the whole catalogue is 119,626 rows, so this is about 1.4 MB. The object
// that multiplies without limit here is the PROBE, not the star -- and giving
// systems something to run out of is precisely what stops the probes doing so.
class Star
{
public:
	Star(uint32_t ID, float worldX, float worldY, float worldZ, const std::string &name,
		 const sf::Color &colour, float displayBrightness);
	// copy/move support needed because of the atomic members
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

	// --- resources -----------------------------------------------------------
	// Set the system's starting stock, and remember it so a later run can restore
	// it. Stocks deplete, so without the second copy run 2 would start in the
	// exhausted galaxy run 1 left behind -- the same trap as the explored flag.
	void setInitialResources(const Resources &r);
	void restoreResources();

	Resources getResources() const;
	Resources getInitialResources() const { return initialStock; }
	bool isExhausted() const;
	// Fraction of the original stock still present, 0..1. Purely for display.
	float remainingFraction() const;

	// Take up to `want` of each resource, and return what was actually available.
	// Probes are updated on several threads at once, so two of them can mine the
	// same system in the same tick; this settles that with a compare-exchange per
	// resource rather than a lock, and the loser simply gets less.
	//
	// const, with mutable stocks. The quadtree hands out `const Star *`, and that
	// constness is about the star's identity -- where it is, what it is called, what
	// colour it burns -- none of which mining changes. This is the case `mutable`
	// exists for: state that is shared between threads and does not form part of the
	// object's observable identity.
	Resources extract(const Resources &want) const;

private:
	static float atomicSubtractUpTo(std::atomic<float> &stock, float want);

	uint32_t ID = 0;
	float worldX = 0.0f;
	float worldY = 0.0f;
	float worldZ = 0.0f;
	std::string name;
	sf::Color colour;
	float displayBrightness = 1.0f;
	std::atomic<bool> isExplored;

	mutable std::atomic<float> metals{0.0f};
	mutable std::atomic<float> volatiles{0.0f};
	mutable std::atomic<float> fissiles{0.0f};
	Resources initialStock;
};

#endif // STAR_H
