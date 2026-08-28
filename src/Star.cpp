// Star.cpp
#include "Star.h"
#include <algorithm>

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
	  isExplored(other.isExplored.load()),
	  metals(other.metals.load()), volatiles(other.volatiles.load()),
	  fissiles(other.fissiles.load()), initialStock(other.initialStock)
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
		metals.store(other.metals.load());
		volatiles.store(other.volatiles.load());
		fissiles.store(other.fissiles.load());
		initialStock = other.initialStock;
	}
	return *this;
}

Star::Star(Star &&other) noexcept
	: ID(other.ID), worldX(other.worldX), worldY(other.worldY), worldZ(other.worldZ),
	  name(std::move(other.name)), colour(other.colour), displayBrightness(other.displayBrightness),
	  isExplored(other.isExplored.load()),
	  metals(other.metals.load()), volatiles(other.volatiles.load()),
	  fissiles(other.fissiles.load()), initialStock(other.initialStock)
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
		metals.store(other.metals.load());
		volatiles.store(other.volatiles.load());
		fissiles.store(other.fissiles.load());
		initialStock = other.initialStock;
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

void Star::setInitialResources(const Resources &r)
{
	initialStock = r;
	metals.store(r.metals);
	volatiles.store(r.volatiles);
	fissiles.store(r.fissiles);
}

void Star::restoreResources()
{
	metals.store(initialStock.metals);
	volatiles.store(initialStock.volatiles);
	fissiles.store(initialStock.fissiles);
}

Resources Star::getResources() const
{
	return Resources(metals.load(), volatiles.load(), fissiles.load());
}

bool Star::isExhausted() const
{
	return getResources().empty();
}

float Star::remainingFraction() const
{
	const float start = initialStock.total();
	if (start <= 0.0f)
		return 0.0f;
	return std::min(1.0f, std::max(0.0f, getResources().total() / start));
}

// Subtract as much of `want` as the stock allows, atomically, and report how much
// was taken. std::atomic<float> has no fetch_sub before C++20, so this is the
// compare-exchange loop that fetch_sub would have been. Contention is low -- it
// only spins when two probes mine the same system on the same tick.
float Star::atomicSubtractUpTo(std::atomic<float> &stock, float want)
{
	if (want <= 0.0f)
		return 0.0f;

	float current = stock.load(std::memory_order_relaxed);
	for (;;)
	{
		if (current <= 0.0f)
			return 0.0f;
		const float taken = std::min(want, current);
		const float remaining = current - taken;
		if (stock.compare_exchange_weak(current, remaining,
										std::memory_order_relaxed, std::memory_order_relaxed))
		{
			return taken;
		}
		// current has been refreshed with the value another thread left; try again.
	}
}

Resources Star::extract(const Resources &want) const
{
	return Resources(atomicSubtractUpTo(metals, want.metals),
					 atomicSubtractUpTo(volatiles, want.volatiles),
					 atomicSubtractUpTo(fissiles, want.fissiles));
}
