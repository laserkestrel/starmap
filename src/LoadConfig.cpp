// LoadConfig.cpp
#include "LoadConfig.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

LoadConfig& LoadConfig::getInstance(const std::string& filename)
{
	static LoadConfig instance(filename);
	return instance;
}

LoadConfig::LoadConfig(const std::string& filename)
{
	loadFromFile(filename);
}

int LoadConfig::getScaleFactor() const
{
	return scaleFactor;
}

int LoadConfig::getWindowWidth() const
{
	return windowWidth;
}

int LoadConfig::getWindowHeight() const
{
	return windowHeight;
}

int LoadConfig::getSleepTimeMillis() const
{
	return sleepTimeMillis;
}

int LoadConfig::getSimulationIterations() const
{
	return simulationIterations;
}

unsigned int LoadConfig::getWorldSeed() const
{
	return worldSeed;
}

void LoadConfig::loadFromFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	try
	{
		json config;
		file >> config;

		// Check if the keys exist before accessing
		if (config.contains("scaleFactor") && config["scaleFactor"].is_number())
		{
			scaleFactor = config["scaleFactor"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid scaleFactor in the config file." << std::endl;
		}

		if (config.contains("window") && config["window"].is_object())
		{
			auto& window = config["window"];
			if (window.contains("width") && window["width"].is_number())
			{
				windowWidth = window["width"];
			}
			else
			{
				std::cerr << "Error: Missing or invalid window width in the config file." << std::endl;
			}

			if (window.contains("height") && window["height"].is_number())
			{
				windowHeight = window["height"];
			}
			else
			{
				std::cerr << "Error: Missing or invalid window height in the config file." << std::endl;
			}
		}
		else
		{
			std::cerr << "Error: Missing or invalid 'window' object in the config file." << std::endl;
		}

		if (config.contains("sleepTimeMillis") && config["sleepTimeMillis"].is_number())
		{
			sleepTimeMillis = config["sleepTimeMillis"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid sleepTimeMillis in the config file." << std::endl;
		}
		if (config.contains("simulationIterations") && config["simulationIterations"].is_number())
		{
			simulationIterations = config["simulationIterations"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid simulationIterations in the config file." << std::endl;
		}
		if (config.contains("worldSeed") && config["worldSeed"].is_number())
		{
			worldSeed = config["worldSeed"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid worldSeed in the config file." << std::endl;
		}
	}
	catch (json::parse_error& e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
}
