// LoadCSVData.h
#ifndef LOADCSVDATA_H
#define LOADCSVDATA_H

#include "Star.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include "LoadConfig.h"
#include <vector>

class LoadCSVData
{
public:
	std::vector<Star> loadStarsFromCsv(const std::string &csvFilePath, sf::RenderWindow &window, const LoadConfig &config);

	// --- Star colour ------------------------------------------------------------
	// Deliberately public: these are pure functions, so they can be unit tested
	// without a window, a config file, or the 34MB catalogue on disk.

	// Ballesteros' formula (2012): effective temperature in Kelvin from the B-V
	// colour index. Good to a few percent across normal stellar types.
	static float colourIndexToTemperatureKelvin(float colourIndex);

	// Physically derived star colour: integrates the Planck blackbody spectrum
	// against the CIE 1931 colour matching functions and converts to sRGB.
	// Returned at full brightness -- magnitude is applied separately.
	static sf::Color temperatureToColour(float kelvin);

	// Fallback for rows with no usable B-V: the typical effective temperature
	// of each spectral class, run through the same pipeline.
	static sf::Color spectralTypeToColour(const std::string &stellarType);

	// Display brightness for an apparent magnitude. NOT physical: true flux spans
	// six orders of magnitude and would render almost every star black.
	static float displayBrightnessForMagnitude(float apparentMagnitude);

	// Scales a colour's brightness while preserving its hue.
	static sf::Color applyBrightness(const sf::Color &colour, float brightness);
};

#endif // LOADCSVDATA_H
