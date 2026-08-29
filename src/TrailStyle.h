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

// --- lineage colouring -------------------------------------------------------
//
// A probe inherits its parent's hue and shifts it by an amount that HALVES with
// every generation. That one detail is what makes the picture readable: the seed's
// own children land far apart on the colour wheel, their children a little closer,
// and by the eighth generation a descendant is within a degree or so of its parent.
// So distance around the wheel approximates distance through the family tree, and a
// glance at the map shows which family owns which part of the galaxy.
//
// The alternative -- a fixed random step per generation -- is a random walk. Its
// spread grows without bound, so after a dozen generations unrelated families land
// on the same hue and the colour stops meaning anything.
inline float childLineageHue(float parentHue, int childIndex, int maxChildren, int parentGeneration)
{
	const int slots = std::max(1, maxChildren) + 1;
	const float slot = static_cast<float>(childIndex + 1) / static_cast<float>(slots);

	// Past this depth the step is far below what the eye can separate, so it stops
	// shrinking -- otherwise it underflows and every deep descendant is identical.
	const int depth = std::min(parentGeneration, 8);
	const float shrink = static_cast<float>(1 << depth);

	float hue = parentHue + slot / shrink;
	hue -= std::floor(hue); // wrap to 0..1
	return hue;
}

// Hue 0..1 to a bright, saturated colour. Saturation and value are held high and
// constant so that only the HUE carries meaning -- brightness is doing a different
// job in the other modes, and mixing the two would make neither readable.
inline sf::Color lineageColour(float hue, float alpha = 235.0f)
{
	const float h = (hue - std::floor(hue)) * 6.0f;
	const int sector = static_cast<int>(h) % 6;
	const float f = h - std::floor(h);

	const float v = 1.0f;      // value
	const float s = 0.78f;     // a little off full saturation reads better on black
	const float p = v * (1.0f - s);
	const float q = v * (1.0f - s * f);
	const float t = v * (1.0f - s * (1.0f - f));

	float r = v, g = v, b = v;
	switch (sector)
	{
	case 0: r = v; g = t; b = p; break;
	case 1: r = q; g = v; b = p; break;
	case 2: r = p; g = v; b = t; break;
	case 3: r = p; g = q; b = v; break;
	case 4: r = t; g = p; b = v; break;
	default: r = v; g = p; b = q; break;
	}

	return sf::Color(static_cast<sf::Uint8>(r * 255.0f),
					 static_cast<sf::Uint8>(g * 255.0f),
					 static_cast<sf::Uint8>(b * 255.0f),
					 static_cast<sf::Uint8>(std::max(0.0f, std::min(255.0f, alpha))));
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
