// TrailStyle.h
#pragma once

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>
#include <string>

// How a probe trail is coloured.
//
// It used to be one random colour per probe, assigned at birth. That looks busy and
// tells you nothing: siblings get unrelated hues, a parent and its child get
// unrelated hues, and with a fleet in the thousands the map becomes confetti. The
// two modes below spend the same pixels on something you can actually read.
enum class TrailColourMode
{
	Recency = 0, // how long ago the leg was flown -- the expansion wavefront
	Density,     // how many probes have arrived at that system -- where the waste is
	Lineage,     // which family flew it -- related probes share a hue
	Trait,       // a genome value -- watch a strategy spread or die out
	ModeCount
};

const char *trailColourModeName(TrailColourMode mode);
TrailColourMode trailColourModeFromString(const std::string &name);

// Eight ramps to choose from at runtime. Each is defined by one mid-tone: the ramp
// runs from a dark version of it, through the colour itself, up to near-white at the
// hot end. Keeping it single-hue means the brightness ordering stays readable --
// rainbow ramps look livelier and are much harder to rank by eye.
struct TrailPalette
{
	const char *name;
	sf::Uint8 r, g, b;
};

const TrailPalette *trailPalettes();
int trailPaletteCount();

// t is 0 (cold) to 1 (hot).
inline sf::Color heatColour(float t, const TrailPalette &palette)
{
	t = std::max(0.0f, std::min(1.0f, t));

	const float br = static_cast<float>(palette.r);
	const float bg = static_cast<float>(palette.g);
	const float bb = static_cast<float>(palette.b);

	float r, g, b;
	if (t < 0.5f)
	{
		// Dark end up to the palette colour itself.
		const float k = t * 2.0f;
		r = br * (0.12f + 0.88f * k);
		g = bg * (0.12f + 0.88f * k);
		b = bb * (0.12f + 0.88f * k);
	}
	else
	{
		// Palette colour up towards white, so the hottest legs read as glowing
		// rather than merely saturated.
		const float k = (t - 0.5f) * 2.0f;
		r = br + (255.0f - br) * k;
		g = bg + (255.0f - bg) * k;
		b = bb + (255.0f - bb) * k;
	}

	// Trails are drawn with additive blending, so the cold end has to actually be
	// dark or the whole field washes out into a haze.
	const float alpha = 60.0f + 195.0f * t;

	return sf::Color(static_cast<sf::Uint8>(std::min(255.0f, r)),
					 static_cast<sf::Uint8>(std::min(255.0f, g)),
					 static_cast<sf::Uint8>(std::min(255.0f, b)),
					 static_cast<sf::Uint8>(std::min(255.0f, alpha)));
}

// Density counts are heavy-tailed -- a handful of systems take an enormous number of
// arrivals while most take a few -- so a linear scale would leave almost everything
// at the cold end. Log compresses it into something with visible structure.
inline float densityHeat(unsigned int arrivals, float saturateAt)
{
	if (arrivals <= 1)
		return 0.0f;
	const float denominator = std::log(1.0f + std::max(2.0f, saturateAt));
	return std::min(1.0f, std::log(1.0f + static_cast<float>(arrivals)) / denominator);
}

// Recent legs are hot and fade to cold over fadeTicks.
inline float recencyHeat(long long segmentTick, long long currentTick, float fadeTicks)
{
	if (fadeTicks <= 0.0f)
		return 1.0f;
	const float age = static_cast<float>(currentTick - segmentTick);
	if (age <= 0.0f)
		return 1.0f;
	return std::max(0.0f, 1.0f - age / fadeTicks);
}
