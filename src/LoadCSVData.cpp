// LoadCSVData.cpp

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <limits>
#include "LoadCSVData.h"
#include <cmath>
#include <cctype>
#include <SFML/Graphics/RenderWindow.hpp>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

namespace
{
	// --- Display tuning ---------------------------------------------------------
	// Star colour below is physically derived and has no tuning. These control how
	// apparent magnitude maps to screen brightness, which is a display decision:
	// real flux spans about six orders of magnitude over this catalogue, so a
	// physical falloff renders all but a handful of stars as black pixels.
	const float BRIGHTEST_MAGNITUDE = -1.5f; // at or below this, full brightness
	const float FAINTEST_MAGNITUDE = 12.0f;  // at or above this, the floor below
	const float MINIMUM_BRIGHTNESS = 0.35f;  // keeps faint stars dim but still coloured

	// Saturation lift applied about the neutral axis, after the physically derived
	// colour is computed. It scales how far each star sits from grey; it does NOT
	// change the hue, so the ordering blue -> white -> yellow -> orange -> red stays
	// exactly as the blackbody maths produced it.
	//
	// 1.0 is physically accurate, but real stars are far less saturated than popular
	// imagery suggests -- most are near-white with only a faint tint, which reads as
	// washed out at the three or four pixels a star occupies here. 3.0 is a stylised
	// setting chosen so the warm/cool spread is legible at that size.
	const float COLOUR_SATURATION = 3.0f;

	// Removes the surrounding double quotes that the HYG catalogue puts around some
	// fields. Without this a quoted spectral type like "G2V" has '"' as its first
	// character and falls through to the unknown-class fallback -- which accounted
	// for roughly 3.9% of the catalogue.
	std::string stripQuotes(const std::string &field)
	{
		std::size_t begin = 0;
		std::size_t end = field.size();
		while (begin < end && (field[begin] == '"' || field[begin] == ' '))
			++begin;
		while (end > begin && (field[end - 1] == '"' || field[end - 1] == ' '))
			--end;
		return field.substr(begin, end - begin);
	}

	// Piecewise Gaussian used by the CIE colour matching function fits.
	float piecewiseGaussian(float x, float mu, float sigmaLeft, float sigmaRight)
	{
		const float sigma = (x < mu) ? sigmaLeft : sigmaRight;
		const float t = (x - mu) / sigma;
		return std::exp(-0.5f * t * t);
	}

	// Wyman, Sloan & Shirley (2013), "Simple Analytic Approximations to the CIE XYZ
	// Colour Matching Functions". Multi-lobe Gaussian fits, wavelength in nanometres.
	void cieColourMatch(float nm, float &x, float &y, float &z)
	{
		x = 1.056f * piecewiseGaussian(nm, 599.8f, 37.9f, 31.0f) +
			0.362f * piecewiseGaussian(nm, 442.0f, 16.0f, 26.7f) -
			0.065f * piecewiseGaussian(nm, 501.1f, 20.4f, 26.2f);
		y = 0.821f * piecewiseGaussian(nm, 568.8f, 46.9f, 40.5f) +
			0.286f * piecewiseGaussian(nm, 530.9f, 16.3f, 31.1f);
		z = 1.217f * piecewiseGaussian(nm, 437.0f, 11.8f, 36.0f) +
			0.681f * piecewiseGaussian(nm, 459.0f, 26.0f, 13.8f);
	}

	// Planck's law: spectral radiance of a blackbody. Only the shape matters here,
	// since the result is normalised, so the leading constants could be dropped.
	double planckSpectralRadiance(double nm, double kelvin)
	{
		const double lambda = nm * 1e-9;
		const double h = 6.62607015e-34;
		const double c = 2.99792458e8;
		const double k = 1.380649e-23;
		return (2.0 * h * c * c) / std::pow(lambda, 5.0) /
			   std::expm1(h * c / (lambda * k * kelvin));
	}

	// Linear light -> sRGB, including the gamma transfer function.
	sf::Uint8 encodeSrgb(float linear)
	{
		linear = std::max(0.0f, std::min(1.0f, linear));
		const float encoded = (linear <= 0.0031308f)
								  ? (12.92f * linear)
								  : (1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f);
		return static_cast<sf::Uint8>(std::lround(255.0f * std::max(0.0f, std::min(1.0f, encoded))));
	}
} // namespace

float LoadCSVData::colourIndexToTemperatureKelvin(float colourIndex)
{
	// Ballesteros (2012). Clamp the input so wild catalogue values can't produce a
	// negative or divide-by-zero temperature.
	const float bv = std::max(-0.4f, std::min(2.5f, colourIndex));
	const float t = 4600.0f * (1.0f / (0.92f * bv + 1.7f) + 1.0f / (0.92f * bv + 0.62f));
	return std::max(1000.0f, std::min(40000.0f, t));
}

sf::Color LoadCSVData::temperatureToColour(float kelvin)
{
	kelvin = std::max(1000.0f, std::min(40000.0f, kelvin));

	// Integrate the blackbody spectrum against the CIE matching functions.
	double X = 0.0, Y = 0.0, Z = 0.0;
	for (double nm = 360.0; nm <= 830.0; nm += 5.0)
	{
		const double radiance = planckSpectralRadiance(nm, kelvin);
		float cx = 0.0f, cy = 0.0f, cz = 0.0f;
		cieColourMatch(static_cast<float>(nm), cx, cy, cz);
		X += radiance * cx;
		Y += radiance * cy;
		Z += radiance * cz;
	}

	const double sum = X + Y + Z;
	if (sum <= 0.0)
	{
		return sf::Color::White;
	}
	X /= sum;
	Y /= sum;
	Z /= sum;

	// CIE XYZ -> linear sRGB (D65 primaries).
	float r = static_cast<float>(3.2406 * X - 1.5372 * Y - 0.4986 * Z);
	float g = static_cast<float>(-0.9689 * X + 1.8758 * Y + 0.0415 * Z);
	float b = static_cast<float>(0.0557 * X - 0.2040 * Y + 1.0570 * Z);

	r = std::max(0.0f, r);
	g = std::max(0.0f, g);
	b = std::max(0.0f, b);

	// Normalise to full brightness -- magnitude is applied separately.
	const float peak = std::max(r, std::max(g, b));
	if (peak > 0.0f)
	{
		r /= peak;
		g /= peak;
		b /= peak;
	}

	// Optional saturation lift about the neutral axis. At 1.0 this is a no-op.
	if (COLOUR_SATURATION != 1.0f)
	{
		const float grey = 0.2126f * r + 0.7152f * g + 0.0722f * b;
		r = grey + (r - grey) * COLOUR_SATURATION;
		g = grey + (g - grey) * COLOUR_SATURATION;
		b = grey + (b - grey) * COLOUR_SATURATION;
	}

	return sf::Color(encodeSrgb(r), encodeSrgb(g), encodeSrgb(b));
}

sf::Color LoadCSVData::spectralTypeToColour(const std::string &stellarType)
{
	// Typical effective temperature per class, put through the same physical
	// pipeline as the B-V path so the two agree.
	float kelvin = 5000.0f; // unknown class: an ordinary mid-range star, not grey

	const std::string cleaned = stripQuotes(stellarType);
	if (!cleaned.empty())
	{
		// The catalogue mixes case ("kA5", "dM2", "m"), so normalise first --
		// the previous uppercase-only comparisons dropped about 850 stars.
		const char firstChar = static_cast<char>(std::toupper(static_cast<unsigned char>(cleaned[0])));
		switch (firstChar)
		{
		case 'O': kelvin = 35000.0f; break;
		case 'B': kelvin = 15000.0f; break;
		case 'A': kelvin = 8500.0f;  break;
		case 'F': kelvin = 6600.0f;  break;
		case 'G': kelvin = 5700.0f;  break;
		case 'K': kelvin = 4400.0f;  break;
		case 'M': kelvin = 3200.0f;  break;
		case 'D': kelvin = 10000.0f; break; // white dwarf
		default: break;
		}
	}

	return temperatureToColour(kelvin);
}

float LoadCSVData::displayBrightnessForMagnitude(float apparentMagnitude)
{
	// Linear in magnitude rather than in flux. Magnitude is already logarithmic in
	// flux, so this is roughly perceptual and keeps the faint majority visible.
	const float span = FAINTEST_MAGNITUDE - BRIGHTEST_MAGNITUDE;
	float t = (FAINTEST_MAGNITUDE - apparentMagnitude) / span;
	t = std::max(0.0f, std::min(1.0f, t));
	return MINIMUM_BRIGHTNESS + (1.0f - MINIMUM_BRIGHTNESS) * t;
}

sf::Color LoadCSVData::applyBrightness(const sf::Color &colour, float brightness)
{
	brightness = std::max(0.0f, std::min(1.0f, brightness));
	return sf::Color(static_cast<sf::Uint8>(colour.r * brightness),
					 static_cast<sf::Uint8>(colour.g * brightness),
					 static_cast<sf::Uint8>(colour.b * brightness),
					 colour.a);
}

std::vector<Star> LoadCSVData::loadStarsFromCsv(const std::string &csvFilePath, sf::RenderWindow &window, const LoadConfig &config)
{
	std::vector<Star> stars;
	std::ifstream csvFile(csvFilePath);
	std::string line;

	sf::Vector2u windowSize = window.getSize();

	float center_x = windowSize.x / 2.0f;
	float center_y = windowSize.y / 2.0f;

	const float dataScalingFactor = static_cast<float>(config.getScaleFactor()); // The config scale value is how many parsecs you want to view on screen.
	const float syntheticScalingFactor = windowSize.x / dataScalingFactor;		// The data is then plotted X + Y to scale into the available resolution.
	const float scaling_factor_x = syntheticScalingFactor;
	const float scaling_factor_y = syntheticScalingFactor; // Keep same as X so to keep map "square"

	// Define the column indices based on your CSV structure (The code indexes first column as zero)
	const int NAME_INDEX0 = 0;					// "id" column for the id value
	const int NAME_INDEX6 = 6;					// "proper" column for the name
	const int NAME_INDEX7 = 7;					// "ra" column for the right ascension (Hours)
	[[maybe_unused]] const int NAME_INDEX8 = 8;	// "dec" column for the declination (not yet used)
	const int NAME_INDEX9 = 9;					// "dist" column for the distance (Parsecs)
	const int NAME_INDEX13 = 13;				// "mag" column for the apparent magnitude (visibility from earth)
	const int NAME_INDEX15 = 15;				// "spect" column for the spectral type (K, M etc)
	const int NAME_INDEX16 = 16;				// "ci" column for the B-V colour index

	if (!csvFile.is_open())
	{
		std::cerr << "Error opening CSV file: " << csvFilePath << std::endl;
		return stars;
	}

	// Read all records into memory, parse the distance column, then sort by distance and load the nearest N
	std::vector<std::vector<std::string>> allRecords;

	// Skip the header line
	std::getline(csvFile, line);

	while (std::getline(csvFile, line))
	{
		std::stringstream ss(line);
		std::string field;
		std::vector<std::string> fields;

		// Split the line into comma-separated fields
		while (getline(ss, field, ','))
		{
			fields.push_back(field);
		}

		if (!fields.empty())
		{
			allRecords.push_back(std::move(fields));
		}
	}

	csvFile.close();

	// Prepare a vector of indices sorted by distance (NAME_INDEX9)
	std::vector<size_t> indices(allRecords.size());
	for (size_t i = 0; i < indices.size(); ++i)
		indices[i] = i;

	auto parseDistance = [&](const std::vector<std::string> &fields) -> float {
		try
		{
			if (fields.size() > static_cast<size_t>(NAME_INDEX9))
				return std::stof(fields[NAME_INDEX9]);
		}
		catch (...) {}
		return std::numeric_limits<float>::infinity();
	};

	std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
		return parseDistance(allRecords[a]) < parseDistance(allRecords[b]);
	});

	int dataLoaderStarsLimit = config.getLoadStarsLimit();
	size_t limit = indices.size();
	if (dataLoaderStarsLimit > 0 && static_cast<size_t>(dataLoaderStarsLimit) < limit)
		limit = static_cast<size_t>(dataLoaderStarsLimit);

	for (size_t idx = 0; idx < limit; ++idx)
	{
		const auto &fields = allRecords[indices[idx]];
		if (fields.size() <= static_cast<size_t>(NAME_INDEX16))
			continue;

		// Assign the id column as star unique identifier. needs converting from string to uint32.
		uint32_t newStarID = 0;

		try
		{
			newStarID = static_cast<uint32_t>(std::stoul(stripQuotes(fields[NAME_INDEX0])));
		}
		catch (const std::exception &e)
		{
			std::cerr << "Invalid/Out of range ID: " << e.what() << std::endl;
		}

		// Get the Name of the Star from column 7 or others.
		std::string newStarName = stripQuotes(fields[NAME_INDEX6]);

		if (newStarName.empty())
		{
			for (int i = 6; i >= 2; --i)
			{
				const std::string candidate = stripQuotes(fields[i]);
				if (!candidate.empty())
				{
					newStarName = candidate;
					break;
				}
			}
		}

		float starAppMagnitude = 0.0f;
		try
		{
			starAppMagnitude = std::stof(fields[NAME_INDEX13]);
		}
		catch (...) {}

		// Prefer the measured B-V colour index -- it is a continuous quantity and
		// gives a far better colour than the first letter of the spectral type.
		// Fall back to the spectral class only when B-V is missing or unparseable.
		sf::Color rawStarColor;
		bool haveColourIndex = false;
		const std::string colourIndexField = stripQuotes(fields[NAME_INDEX16]);
		if (!colourIndexField.empty())
		{
			try
			{
				const float bv = std::stof(colourIndexField);
				rawStarColor = temperatureToColour(colourIndexToTemperatureKelvin(bv));
				haveColourIndex = true;
			}
			catch (...) {}
		}
		if (!haveColourIndex)
		{
			rawStarColor = spectralTypeToColour(fields[NAME_INDEX15]);
		}

		const sf::Color adjStarColor = applyBrightness(rawStarColor, displayBrightnessForMagnitude(starAppMagnitude));

		float ra_rad = 0.0f;
		try
		{
			ra_rad = (6.0f - std::stof(fields[NAME_INDEX7])) * (2.0f * static_cast<float>(M_PI) / 24.0f);
		}
		catch (...) {}

		float distance_parsecs = parseDistance(fields);

		float star_x = center_x + distance_parsecs * std::cos(ra_rad) * scaling_factor_x;
		float star_y = center_y + distance_parsecs * std::sin(ra_rad) * scaling_factor_y;

		stars.emplace_back(newStarID, static_cast<int>(star_x), static_cast<int>(star_y), newStarName, adjStarColor);
	}

	return stars;
}
