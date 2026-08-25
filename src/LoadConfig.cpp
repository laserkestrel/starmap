// LoadConfig.cpp
#include "LoadConfig.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	std::string configFilename = "./content/config.json";

	// Each reader leaves the target untouched when the key is absent or the wrong
	// type, so the defaults declared in the header stand.
	template <typename T>
	void readNumber(const json &node, const char *key, T &target)
	{
		if (node.contains(key) && node[key].is_number())
		{
			target = node[key].get<T>();
		}
		else
		{
			std::cerr << "Config: missing or invalid '" << key << "', keeping default " << target << std::endl;
		}
	}

	// Accepts a real JSON boolean, and still tolerates the legacy "true"/"false"
	// strings that earlier versions of config.json used.
	void readBool(const json &node, const char *key, bool &target)
	{
		if (node.contains(key))
		{
			if (node[key].is_boolean())
			{
				target = node[key].get<bool>();
				return;
			}
			if (node[key].is_string())
			{
				const std::string value = node[key].get<std::string>();
				if (value == "true") { target = true; return; }
				if (value == "false") { target = false; return; }
			}
		}
		std::cerr << "Config: missing or invalid '" << key << "', keeping default "
				  << (target ? "true" : "false") << std::endl;
	}
} // namespace

LoadConfig &LoadConfig::getInstance()
{
	static LoadConfig instance;
	return instance;
}

LoadConfig::LoadConfig()
{
	loadFromFile();
}

int LoadConfig::getScaleFactor() const { return scaleFactor; }
int LoadConfig::getWindowWidth() const { return windowWidth; }
int LoadConfig::getWindowHeight() const { return windowHeight; }
int LoadConfig::getSleepTimeMillis() const { return sleepTimeMillis; }
int LoadConfig::getSimulationIterations() const { return simulationIterations; }
int LoadConfig::getLoadStarsLimit() const { return loadStarsLimit; }
int LoadConfig::getQuadTreeSearchSize() const { return quadtreeSearchSize; }
bool LoadConfig::getSummaryShowPerProbe() const { return summaryShowPerProbe; }
bool LoadConfig::getSummaryShowFooter() const { return summaryShowFooter; }
int LoadConfig::getprobeIndividualReplicationLimit() const { return probeIndividualReplicationLimit; }
float LoadConfig::getProbeSearchRadiusParsecs() const { return probeSearchRadiusParsecs; }
float LoadConfig::getProbeSpeedParsecsPerTick() const { return probeSpeedParsecsPerTick; }
float LoadConfig::getViewTiltDegrees() const { return viewTiltDegrees; }
float LoadConfig::getViewDepthParsecs() const { return viewDepthParsecs; }

void LoadConfig::loadFromFile()
{
	std::ifstream file(configFilename);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << configFilename << " -- using built-in defaults." << std::endl;
		return;
	}

	try
	{
		json config;
		file >> config;

		readNumber(config, "scaleFactor", scaleFactor);

		if (config.contains("window") && config["window"].is_object())
		{
			const auto &window = config["window"];
			readNumber(window, "width", windowWidth);
			readNumber(window, "height", windowHeight);
		}
		else
		{
			std::cerr << "Config: missing or invalid 'window' object, keeping defaults." << std::endl;
		}

		readNumber(config, "sleepTimeMillis", sleepTimeMillis);
		readNumber(config, "simulationIterations", simulationIterations);
		readNumber(config, "loadStarsLimit", loadStarsLimit);
		readNumber(config, "quadtreeSearchSize", quadtreeSearchSize);
		readNumber(config, "probeIndividualReplicationLimit", probeIndividualReplicationLimit);
		readNumber(config, "probeSearchRadiusParsecs", probeSearchRadiusParsecs);
		readNumber(config, "probeSpeedParsecsPerTick", probeSpeedParsecsPerTick);
		readNumber(config, "viewTiltDegrees", viewTiltDegrees);
		readNumber(config, "viewDepthParsecs", viewDepthParsecs);
		readBool(config, "summaryShowPerProbe", summaryShowPerProbe);
		readBool(config, "summaryShowFooter", summaryShowFooter);
	}
	catch (json::parse_error &e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << " -- using built-in defaults." << std::endl;
	}
}
