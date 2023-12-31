// LoadCSVData.cpp

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
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

	int loadedStars = 0; // Counter for the number of loaded stars
	int dataLoaderStarsLimit;
	dataLoaderStarsLimit = config.getLoadStarsLimit();

	// Skip the header line
	std::getline(csvFile, line);

	while (std::getline(csvFile, line) && loadedStars < dataLoaderStarsLimit)
	{
		std::stringstream ss(line);
		std::string field;
		std::vector<std::string> fields;

		// Split the line into comma-separated fields
		while (getline(ss, field, ','))
		{
			fields.push_back(field);
		}

		// Extract and convert fields

		// Assign the id column as star unique identifier. needs converting from string to uint32.
		// std::cout << "Field column Value: " << fields[0] << std::endl; // useful debug.
		std::string newStarIDAsString = fields[NAME_INDEX0];
		uint32_t newStarID;

		try
		{
			newStarID = std::stoul(newStarIDAsString);
		}
		catch (const std::invalid_argument &e)
		{
			std::cerr << "Invalid argument: " << e.what() << std::endl;
		}
		catch (const std::out_of_range &e)
		{
			std::cerr << "Out of range: " << e.what() << std::endl;
		}

		// Get the Name of the Star from column 7 or others.
		std::string newStarName = fields[NAME_INDEX6];

		// If column 7's value is "", check columns 2 through 6 for a non-empty string to use as the name
		if (newStarName == "\"\"")
		{
			for (int i = 6; i >= 2; --i)
			{
				if (fields[i] != "\"\"")
				{
					newStarName = fields[i];
					// std::cout << "Found a non blank alternative name to use: " << newStarName << std::endl; // TOO VERBOSE
					break; // Found a non-empty name, break the loop
				}
			}
		}

		// Convert Spectral Type to RGB color using the provided function
		std::string spectralType = fields[NAME_INDEX15]; // Assuming spectral type is in the 16th column
		// std::cout << "Spectral Value from CSV: " << fields[NAME_INDEX15] << std::endl; // TOO VERBOSE

		float starAppMagnitude = std::stof(fields[NAME_INDEX13]);

		sf::Color rawStarColor = convertStellarTypeToColor(spectralType);
		sf::Color adjStarColor = adjustStellarBrightness(rawStarColor, starAppMagnitude);

		// Convert RA to radians (360 degrees = 24 hours)
		// FACT: answer in radians = value_in_hours x (2 * PI / 24)
		// FACT: 6.28 radians in a full circle (2*PI)
		// FACT: 360 degrees = 24 hours)

		// float ra_rad = std::stof(fields[NAME_INDEX7]) * (2.0f * M_PI / 24.0f);
		// do an 18 hour correction to rotate so we have N at upper display, S at bottom, and E+W associated to Right and left.
		float ra_rad = (6.0f - std::stof(fields[NAME_INDEX7])) * (2.0f * M_PI / 24.0f);

		// std::cout << "Input hours/mins value of " << fields[NAME_INDEX7] << " for star ID: " << fields[NAME_INDEX0] << " generates radian value of: " << ra_rad << std::endl; // TOO VERBOSE
		//  Get the distance in parsecs
		float distance_parsecs = std::stof(fields[NAME_INDEX9]);

		// Calculate x and y based on scaling factor
		float star_x = center_x + distance_parsecs * std::cos(ra_rad) * scaling_factor_x;
		float star_y = center_y + distance_parsecs * std::sin(ra_rad) * scaling_factor_y;

		// Create a Star object and add it to the vector
		stars.emplace_back(newStarID, star_x, star_y, newStarName, adjStarColor);

		// Increment the loadedStars counter
		loadedStars++;
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