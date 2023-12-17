// LoadData.h
#ifndef LOAD_DATA_H
#define LOAD_DATA_H

#include "LoadConfig.h"
#include "Star.h"
#include <SFML/Graphics/Color.hpp> // Include SFML Color class
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>

class LoadData
{
public:
	static std::vector<Star> loadStarsFromJson(const std::string& jsonFilePath, sf::RenderWindow& window, const LoadConfig& config); // Modified method signature

private:
	static sf::Color convertStellarTypeToColor(char stellarType);
};

#endif // LOAD_DATA_H
