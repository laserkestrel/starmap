// LoadCSVData.h
#ifndef LOADCSVDATA_H
#define LOADCSVDATA_H

#include "Star.h"
#include <string>

#include <vector>

class LoadCSVData
{
public:
	// Function to load stars from a given CSV file path
	std::vector<Star> loadStarsFromCsv(const std::string &csvFilePath);

private:
	static sf::Color convertStellarTypeToColor(const std::string &stellarType);
	static sf::Color adjustStellarBrightness(const sf::Color &originalColor, float brightnessFactor);
};

#endif // LOADCSVDATA_H
