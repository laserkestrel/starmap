// LoadCSVData.cpp - Pseudo-code for loading stars from CSV

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include "LoadCSVData.h"

std::vector<Star> LoadCSVData::loadStarsFromCsv(const std::string &csvFilePath)
{
	std::vector<Star> stars;
	std::ifstream csvFile(csvFilePath);
	std::string line;

	// Define the column indices based on your CSV structure
	const int NAME_INDEX6 = 6;	 // "proper" column for the name
	const int NAME_INDEX8 = 8;	 // "ra" column for the right assention.
	const int NAME_INDEX9 = 9;	 // "dec" column for the declination
	const int NAME_INDEX10 = 10; // "dist" column for the distance

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
		std::string name = fields[NAME_INDEX6];
		// Handle distance and positional data
		// For simplicity, let's say you calculate x, y from distance or have default values
		int x = 1 /* some calculation or default */;
		int y = 1 /* some calculation or default */;
		sf::Color color = sf::Color(191, 64, 191); /* default or based on spectral type or other criteria */
		;

		// Create a Star object and add it to the vector
		stars.emplace_back(x, y, name, color);
	}

	csvFile.close();
	return stars;
}
