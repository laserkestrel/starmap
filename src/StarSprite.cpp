// StarSprite.cpp
#include "StarSprite.h"
#include <SFML/Graphics/Image.hpp>
#include <algorithm>
#include <cmath>

namespace
{
	float gaussian(float value, float width)
	{
		const float t = value / width;
		return std::exp(-t * t);
	}

	// All four profiles take normalised coordinates in [-1, 1] and return alpha in
	// [0, 1]. They are the same formulas used for the mockups.
	float profile(StarSpriteStyle style, float nx, float ny)
	{
		const float r = std::sqrt(nx * nx + ny * ny);
		switch (style)
		{
		case StarSpriteStyle::SoftGlow:
			return gaussian(r, 0.30f);

		case StarSpriteStyle::CoreHalo:
			return std::min(1.0f, gaussian(r, 0.085f) + 0.42f * gaussian(r, 0.34f));

		case StarSpriteStyle::DiffractionSpikes:
		{
			const float base = gaussian(r, 0.075f) + 0.38f * gaussian(r, 0.30f);
			// Two thin bars crossing at the centre, fading along their length. They
			// only become visible on bright stars, so faint ones stay clean.
			const float horizontal = gaussian(std::abs(ny), 0.020f) * std::exp(-std::abs(nx) / 0.34f) * 0.55f;
			const float vertical = gaussian(std::abs(nx), 0.020f) * std::exp(-std::abs(ny) / 0.34f) * 0.55f;
			return std::min(1.0f, base + horizontal + vertical);
		}

		case StarSpriteStyle::BloomRing:
		{
			const float base = gaussian(r, 0.075f) + 0.30f * gaussian(r, 0.26f);
			const float ring = 0.16f * gaussian(r - 0.52f, 0.075f);
			return std::min(1.0f, base + ring);
		}

		default:
			return gaussian(r, 0.30f);
		}
	}
} // namespace

StarSpriteStyle starSpriteStyleFromString(const std::string &name)
{
	if (name == "softGlow") return StarSpriteStyle::SoftGlow;
	if (name == "coreHalo") return StarSpriteStyle::CoreHalo;
	if (name == "diffractionSpikes") return StarSpriteStyle::DiffractionSpikes;
	if (name == "bloomRing") return StarSpriteStyle::BloomRing;
	return StarSpriteStyle::CoreHalo;
}

const char *starSpriteStyleName(StarSpriteStyle style)
{
	switch (style)
	{
	case StarSpriteStyle::SoftGlow: return "softGlow";
	case StarSpriteStyle::CoreHalo: return "coreHalo";
	case StarSpriteStyle::DiffractionSpikes: return "diffractionSpikes";
	case StarSpriteStyle::BloomRing: return "bloomRing";
	default: return "coreHalo";
	}
}

sf::Texture makeStarSprite(StarSpriteStyle style, unsigned int resolution)
{
	resolution = std::max(8u, resolution);
	sf::Image image;
	image.create(resolution, resolution, sf::Color(255, 255, 255, 0));

	for (unsigned int py = 0; py < resolution; ++py)
	{
		for (unsigned int px = 0; px < resolution; ++px)
		{
			// Sample at pixel centres so the sprite stays symmetric.
			const float nx = ((static_cast<float>(px) + 0.5f) / resolution) * 2.0f - 1.0f;
			const float ny = ((static_cast<float>(py) + 0.5f) / resolution) * 2.0f - 1.0f;
			const float alpha = std::max(0.0f, std::min(1.0f, profile(style, nx, ny)));
			image.setPixel(px, py, sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha * 255.0f)));
		}
	}

	sf::Texture texture;
	texture.loadFromImage(image);
	texture.setSmooth(true);
	return texture;
}
