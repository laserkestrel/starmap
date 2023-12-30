// LoadConfig.cpp
#include "LoadConfig.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	std::string configFilename = "./content/config.json"; // Hardcoded default "/config_path/filename.json"
}

LoadConfig &LoadConfig::getInstance()
{
	static LoadConfig instance;
	return instance;
}

LoadConfig::LoadConfig()
{
	loadFromFile();
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

int LoadConfig::getLoadStarsLimit() const
{
	return loadStarsLimit;
}

unsigned int LoadConfig::getWorldSeed() const
{
	return worldSeed;
}

int LoadConfig::getQuadTreeSearchSize() const
{
	return quadtreeSearchSize;
}

bool LoadConfig::getSummaryShowPerProbe() const
{
	return summaryShowPerProbe;
}

bool LoadConfig::getSummaryShowFooter() const
{
	return summaryShowFooter;
}

int LoadConfig::getprobeIndividualReplicationLimit() const
{
	return probeIndividualReplicationLimit;
}

int LoadConfig::getProbeSearchRadiusPixels() const
{
	return probeSearchRadiusPixels;
}

void LoadConfig::loadFromFile()
{
	// std::string filename = configFilename;
	std::ifstream file(configFilename);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << configFilename << std::endl;
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
			auto &window = config["window"];
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

		if (config.contains("loadStarsLimit") && config["loadStarsLimit"].is_number())
		{
			loadStarsLimit = config["loadStarsLimit"];
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
		if (config.contains("quadtreeSearchSize") && config["quadtreeSearchSize"].is_number())
		{
			quadtreeSearchSize = config["quadtreeSearchSize"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid quadtreeSearchSize in the config file." << std::endl;
		}

		if (config.contains("summaryShowPerProbe") && config["summaryShowPerProbe"].is_string())
		{
			std::string perProbe = config["summaryShowPerProbe"];
			if (perProbe == "true")
			{
				summaryShowPerProbe = true;
			}
			else if (perProbe == "false")
			{
				summaryShowPerProbe = false;
			}
			else
			{
				std::cerr << "Error: Invalid value for summaryShowPerProbe in the config file." << std::endl;
			}
		}
		else
		{
			std::cerr << "Error: Missing or invalid summaryShowPerProbe in the config file." << std::endl;
		}

		if (config.contains("summaryShowFooter") && config["summaryShowFooter"].is_string())
		{
			std::string footer = config["summaryShowFooter"];
			if (footer == "true")
			{
				summaryShowFooter = true;
			}
			else if (footer == "false")
			{
				summaryShowFooter = false;
			}
			else
			{
				std::cerr << "Error: Invalid value for summaryShowFooter in the config file." << std::endl;
			}
		}
		else
		{
			std::cerr << "Error: Missing or invalid summaryShowFooter in the config file." << std::endl;
		}
		if (config.contains("probeIndividualReplicationLimit") && config["probeIndividualReplicationLimit"].is_number())
		{
			probeIndividualReplicationLimit = config["probeIndividualReplicationLimit"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid probeIndividualReplicationLimit in the config file." << std::endl;
		}
		if (config.contains("probeSearchRadiusPixels") && config["probeSearchRadiusPixels"].is_number())
		{
			probeSearchRadiusPixels = config["probeSearchRadiusPixels"];
		}
		else
		{
			std::cerr << "Error: Missing or invalid probeSearchRadiusPixels in the config file." << std::endl;
		}
	}
	catch (json::parse_error &e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
}
