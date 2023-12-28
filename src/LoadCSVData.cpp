// LoadCSVData.cpp

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include "LoadCSVData.h"
#include <cmath>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

std::vector<Star> LoadCSVData::loadStarsFromCsv(const std::string &csvFilePath)
{
	std::vector<Star> stars;
	std::ifstream csvFile(csvFilePath);
	std::string line;

	// Define the center point of your view (e.g., the Solar System)
	const float center_x = 0.0f;			// Adjust this as needed - seems to need setting as 0
	const float center_y = 0.0f;			// Adjust this as needed - seems to need setting as 0
	const float scaling_factor_x = 2560.0f; // Adjust this as needed - set to same as width of screen? (positive)
	const float scaling_factor_y = 1080.0f; // Adjust this as needed - set to same as width of screen? (positive)

	// Define the column indices based on your CSV structure
	const int NAME_INDEX6 = 6;	 // "proper" column for the name
	const int NAME_INDEX8 = 8;	 // "ra" column for the right assention.
	const int NAME_INDEX9 = 9;	 // "dec" column for the declination
	const int NAME_INDEX10 = 10; // "dist" column for the distance
	const int NAME_INDEX14 = 14; // "mag" column for the apparant magnitude (visibility from earth
	const int NAME_INDEX15 = 15; // "spect" column for the spectral type
	const int NAME_INDEX16 = 16; // "ci" column for the colour index

	if (!csvFile.is_open())
	{
		std::cerr << "Error opening CSV file: " << csvFilePath << std::endl;
		return stars;
	}

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

		// Extract and convert fields
		std::string newStarName = fields[NAME_INDEX6];

		// Convert Spectral Type to RGB color using the provided function
		std::string spectralType = fields[NAME_INDEX15]; // Assuming spectral type is in the 16th column
		std::cout << "Spectral Value from CSV: " << fields[NAME_INDEX15] << std::endl;

		float starAppMagnitude = std::stof(fields[NAME_INDEX14]);

		sf::Color rawStarColor = convertStellarTypeToColor(spectralType);
		sf::Color adjStarColor = adjustStellarBrightness(rawStarColor, starAppMagnitude);

		// Convert RA and Dec to radians (assuming they are in degrees)
		float ra_rad = std::stof(fields[NAME_INDEX8]) * (M_PI / 180.0f);
		float dec_rad = std::stof(fields[NAME_INDEX9]) * (M_PI / 180.0f);

		// Calculate x and y based on the center point and scaling factor
		float star_x = (ra_rad - center_x) * scaling_factor_x;
		float star_y = (dec_rad - center_y) * scaling_factor_y;

		// sf::Color color = sf::Color(191, 64, 191); /* default or based on spectral type or other criteria */

		// Create a Star object and add it to the vector
		stars.emplace_back(star_x, star_y, newStarName, adjStarColor);
	}

	sf::Color testrawStarColor = convertStellarTypeToColor("K");
	sf::Color testadjStarColor2 = adjustStellarBrightness(testrawStarColor, 2);
	sf::Color testadjStarColor6 = adjustStellarBrightness(testrawStarColor, 6);
	sf::Color testadjStarColor0 = adjustStellarBrightness(testrawStarColor, 0);
	sf::Color testadjStarColor22 = adjustStellarBrightness(testrawStarColor, 22);
	stars.emplace_back(40, 40, "LOOK HERE RAW", testrawStarColor);
	stars.emplace_back(40, 80, "LOOK HERE ADJ2", testadjStarColor2);
	stars.emplace_back(40, 120, "LOOK HERE ADJ6", testadjStarColor6);
	stars.emplace_back(40, 160, "LOOK HERE ADJ0", testadjStarColor0);
	stars.emplace_back(40, 200, "LOOK HERE ADJ22", testadjStarColor22);
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
	const float brightnessScalingFactor = 100.0f;

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