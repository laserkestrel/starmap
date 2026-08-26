// StarSprite.h
#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <string>

// Every star is drawn as the same small texture, tinted to that star's colour and
// scaled to its brightness. One texture, one draw call for the whole field.
//
// The texture is generated in code rather than shipped as an image file, so the
// style can be changed freely and there is no asset to keep in sync.
enum class StarSpriteStyle
{
	SoftGlow,          // one smooth falloff; quiet and astronomical
	CoreHalo,          // tight bright core inside a wider glow
	DiffractionSpikes, // core, halo and four thin spikes; photographic
	BloomRing,         // core, halo and a faint outer ring
	Count
};

StarSpriteStyle starSpriteStyleFromString(const std::string &name);
const char *starSpriteStyleName(StarSpriteStyle style);

// Builds the sprite. The image is white with a varying alpha channel, which is
// what lets a single texture be tinted to any star colour.
sf::Texture makeStarSprite(StarSpriteStyle style, unsigned int resolution = 64);
