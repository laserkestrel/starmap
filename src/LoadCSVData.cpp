// LoadCSVData.cpp

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include "LoadCSVData.h"
#include <cmath>
#include <SFML/Graphics/RenderWindow.hpp>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

std::vector<Star> LoadCSVData::loadStarsFromCsv(const std::string &csvFilePath, sf::RenderWindow &window, const LoadConfig &config)
{
	std::vector<Star> stars;
	std::ifstream csvFile(csvFilePath);
	std::string line;

	sf::Vector2u windowSize = window.getSize();

	float center_x = windowSize.x / 2.0f;
	float center_y = windowSize.y / 2.0f;

	const float dataScalingFactor = config.getScaleFactor();			   // The config scale value is how many parsecs you want to view on screen.
	const float syntheticScalingFactor = windowSize.x / dataScalingFactor; // The data is then plotted X + Y to scale into the available resolution.
	const float scaling_factor_x = syntheticScalingFactor;
	const float scaling_factor_y = syntheticScalingFactor; // Keep same as X so to keep map "square"

	// Define the column indices based on your CSV structure (The code indexes first column as zero)
	const int NAME_INDEX0 = 0;	 // "id" column for the id value
	const int NAME_INDEX6 = 6;	 // "proper" column for the name
	const int NAME_INDEX7 = 7;	 // "ra" column for the right assention (Hours)
	const int NAME_INDEX8 = 8;	 // "dec" column for the declination
	const int NAME_INDEX9 = 9;	 // "dist" column for the distance (Parsecs)
	const int NAME_INDEX13 = 13; // "mag" column for the apparant magnitude (visibility from earth)
	const int NAME_INDEX15 = 15; // "spect" column for the spectral type (K, M etc)
	const int NAME_INDEX16 = 16; // "ci" column for the colour index

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

		// Assign the id column as star unique identifier. needs converting from string to uint32.
		std::string newStarIDAsString = fields[NAME_INDEX0];
		uint32_t newStarID = 0;

		try
		{
			newStarID = std::stoul(newStarIDAsString);
		}
		catch (const std::exception &e)
		{
			std::cerr << "Invalid/Out of range ID: " << e.what() << std::endl;
		}

		// Get the Name of the Star from column 7 or others.
		std::string newStarName = fields[NAME_INDEX6];

		if (newStarName == "\"\"")
		{
			for (int i = 6; i >= 2; --i)
			{
				if (fields[i] != "\"\"")
				{
					newStarName = fields[i];
					break;
				}
			}
		}

		std::string spectralType = fields[NAME_INDEX15];
		float starAppMagnitude = 0.0f;
		try
		{
			starAppMagnitude = std::stof(fields[NAME_INDEX13]);
		}
		catch (...) {}

		sf::Color rawStarColor = convertStellarTypeToColor(spectralType);
		sf::Color adjStarColor = adjustStellarBrightness(rawStarColor, starAppMagnitude);

		float ra_rad = 0.0f;
		try
		{
			ra_rad = (6.0f - std::stof(fields[NAME_INDEX7])) * (2.0f * M_PI / 24.0f);
		}
		catch (...) {}

		float distance_parsecs = parseDistance(fields);

		float star_x = center_x + distance_parsecs * std::cos(ra_rad) * scaling_factor_x;
		float star_y = center_y + distance_parsecs * std::sin(ra_rad) * scaling_factor_y;

		stars.emplace_back(newStarID, star_x, star_y, newStarName, adjStarColor);
	}

	csvFile.close();
	return stars;
}

sf::Color LoadCSVData::convertStellarTypeToColor(const std::string &stellarType)
{
	// Check if the string is not empty
	if (!stellarType.empty())
	{
		// std::cout << "Convert StellarTypeToColor was passed the string: " << stellarType << std::endl;
		char firstChar = stellarType[0]; // Get the first character of the string

		// Your implementation for converting the first character of stellarType to RGB color goes here
		if (firstChar == 'O')
		{
			return sf::Color(255, 255, 255); // White
		}
		else if (firstChar == 'D')
		{
			return sf::Color(224, 225, 253); // White Blue (White Dwarf)
		}
		else if (firstChar == 'B')
		{
			return sf::Color(173, 216, 230); // Light Blue
		}
		else if (firstChar == 'A')
		{
			return sf::Color(255, 255, 0); // Yellow
		}
		else if (firstChar == 'F')
		{
			return sf::Color(255, 165, 0); // Orange
		}
		else if (firstChar == 'G')
		{
			return sf::Color(255, 140, 0); // Dark Orange
		}
		else if (firstChar == 'K')
		{
			return sf::Color(255, 69, 0); // Red-Orange
		}
		else if (firstChar == 'M')
		{
			return sf::Color(255, 0, 0); // Red
		}
	}

	// Default to Gray if the spectral type is not recognized
	return sf::Color(128, 128, 128);
}

sf::Color LoadCSVData::adjustStellarBrightness(const sf::Color &originalColor, float starAppMagnitude)
{
	// Hardcoded brightness scaling factor (adjust as needed)
	const float brightnessScalingFactor = 100.5f; // TODO - MOVE TO CONFIG

	// Calculate the brightness factor based on magnitude and the scaling factor
	float brightnessFactor = 1.0f / pow(2.512f, starAppMagnitude) * brightnessScalingFactor;

	// Ensure the factor is within a valid range (0 to 1)
	brightnessFactor = std::max(0.0f, std::min(1.0f, brightnessFactor));

	// Scale the RGB channels by the brightness factor
	sf::Uint8 r = static_cast<sf::Uint8>(originalColor.r * brightnessFactor);
	sf::Uint8 g = static_cast<sf::Uint8>(originalColor.g * brightnessFactor);
	sf::Uint8 b = static_cast<sf::Uint8>(originalColor.b * brightnessFactor);

	// Return the adjusted color with the same alpha channel
	return sf::Color(r, g, b, originalColor.a);
}