// LoadCSVData.h
#ifndef LOADCSVDATA_H
#define LOADCSVDATA_H

#include "Star.h"

#include <string>

#include <vector>

class LoadCSVData
{
public:
	std::vector<Star> loadStarsFromCsv(const std::string &csvFilePath, sf::RenderWindow &window, const LoadConfig &config);

private:
	static sf::Color convertStellarTypeToColor(const std::string &stellarType);
	static sf::Color adjustStellarBrightness(const sf::Color &originalColor, float brightnessFactor);
};

#endif // LOADCSVDATA_H
