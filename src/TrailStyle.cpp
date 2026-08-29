// TrailStyle.cpp
#include "TrailStyle.h"

namespace
{
	// Eight mid-tones, chosen to stay apart from each other on a black field and to
	// remain distinguishable once the ramp lightens them. Deliberately no dark blue
	// or deep grey: both vanish against the starfield.
	const TrailPalette PALETTES[] = {
		{"Amber",   255, 150,  40},
		{"Ice",      90, 180, 255},
		{"Emerald",  60, 220, 130},
		{"Magenta", 240,  80, 200},
		{"Crimson", 240,  70,  70},
		{"Violet",  160, 120, 255},
		{"Teal",     50, 210, 210},
		{"Gold",    240, 210,  70},
	};
} // namespace

const TrailPalette *trailPalettes() { return PALETTES; }
int trailPaletteCount() { return static_cast<int>(sizeof(PALETTES) / sizeof(PALETTES[0])); }

const char *trailColourModeName(TrailColourMode mode)
{
	switch (mode)
	{
	case TrailColourMode::Recency: return "recency";
	case TrailColourMode::Density: return "density";
	case TrailColourMode::Lineage: return "lineage";
	case TrailColourMode::Trait: return "trait";
	default: return "recency";
	}
}

TrailColourMode trailColourModeFromString(const std::string &name)
{
	if (name == "density") return TrailColourMode::Density;
	if (name == "trait") return TrailColourMode::Trait;
	// "perProbe" was this mode's name back when the colour was random per probe.
	if (name == "lineage" || name == "perProbe" || name == "perprobe") return TrailColourMode::Lineage;
	return TrailColourMode::Recency;
}
