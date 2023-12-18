// LoadData.cpp
#include "LoadData.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random> // Include the random library

using json = nlohmann::json;

std::vector<Star> LoadData::loadStarsFromJson(const std::string &jsonFilePath, sf::RenderWindow &window, const LoadConfig &config)
{
	std::vector<Star> GalaxyVector;

	std::ifstream jsonFile(jsonFilePath);
	if (!jsonFile.is_open())
	{
		std::cerr << "Error opening JSON file: " << jsonFilePath << std::endl;
		return GalaxyVector;
	}

	// Parse the JSON file
	json jsonData;
	jsonFile >> jsonData;

	int worldSeed = config.getWorldSeed();
	std::mt19937 rng(worldSeed); // Use the worldSeed for random number generation

	// Check if jsonData has the key "star_systems"
	if (jsonData.contains("star_systems"))
	{
		// Access the array using operator[]
		json starSystems = jsonData["star_systems"];
		// Calculate the number of star systems in the JSON data
		size_t numStarSystems = starSystems.size();
		// Reserve memory in GalaxyVector to accommodate all star systems
		GalaxyVector.reserve(numStarSystems + 1); // +1 for the "Sol" star

		// Check if starSystems is an array
		if (starSystems.is_array())
		{
			sf::Vector2u windowSize = window.getSize();

			// Calculate the center coordinates
			int centerX = windowSize.x / 2;
			int centerY = windowSize.y / 2;

			// Create the Sol star using the calculated center coordinates
			Star newStar(centerX, centerY, "Sol", (sf::Color(255, 255, 0)), 300, 300, 300);
			newStar.setIsExplored(true);
			GalaxyVector.push_back(newStar);

			// Calculate the maximum x and y coordinates based on window size
			int maxX = windowSize.x - 20; // Subtracting 20 to keep some margin from the edges
			int maxY = windowSize.y - 20;

			// Iterate through each entry in the JSON file
			for (const auto &entry : starSystems)
			{
				std::string name = entry["name"].get<std::string>();
				char stellarType = entry["stellartype"].get<std::string>().front();
				// Generate random coordinates using the seeded generator
				std::uniform_int_distribution<int> distX(20, maxX);
				std::uniform_int_distribution<int> distY(20, maxY);
				int x = distX(rng);
				int y = distY(rng);

				// Generate random values for metals, polymers, and fuel using the seeded generator
				std::uniform_real_distribution<double> distValues(400.0, 1000.0);
				double metals = distValues(rng);
				double polymers = distValues(rng);
				double fuel = distValues(rng);

				// Convert stellarType to RGB color using the provided function
				sf::Color color = convertStellarTypeToColor(stellarType);

				// Instantiate a new Star object and add it to GalaxyVector
				Star newStar(x, y, name, color, metals, polymers, fuel);

				// GalaxyVector.push_back(newStar); //optimised loading to instead load to a temp vector, then push back to preallocated one
				GalaxyVector.push_back(std::move(newStar));
			}
		}
		else
		{
			std::cerr << "The 'star_systems' key does not contain an array." << std::endl;
		}
	}
	else
	{
		std::cerr << "The JSON data does not contain the key 'star_systems'." << std::endl;
	}

	return GalaxyVector;
}

sf::Color LoadData::convertStellarTypeToColor(char stellarType)
{
	// Your implementation for converting stellarType to RGB color goes here
	if (stellarType == 'O')
	{
		return sf::Color(255, 255, 255); // White
	}
	else if (stellarType == 'B')
	{
		return sf::Color(173, 216, 230); // Light Blue
	}
	else if (stellarType == 'A')
	{
		return sf::Color(255, 255, 0); // Yellow
	}
	else if (stellarType == 'F')
	{
		return sf::Color(255, 165, 0); // Orange
	}
	else if (stellarType == 'G')
	{
		return sf::Color(255, 140, 0); // Dark Orange
	}
	else if (stellarType == 'K')
	{
		return sf::Color(255, 69, 0); // Red-Orange
	}
	else if (stellarType == 'M')
	{
		return sf::Color(255, 0, 0); // Red
	}
	else
	{
		return sf::Color(128, 128, 128); // Default to Gray
	}
}
