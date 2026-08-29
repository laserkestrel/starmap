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
	void readString(const json &node, const char *key, std::string &target)
	{
		if (node.contains(key) && node[key].is_string())
		{
			target = node[key].get<std::string>();
		}
		else
		{
			std::cerr << "Config: missing or invalid '" << key << "', keeping default " << target << std::endl;
		}
	}

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

float LoadConfig::getStartingViewRadiusParsecs() const { return startingViewRadiusParsecs; }
int LoadConfig::getWindowWidth() const { return windowWidth; }
int LoadConfig::getWindowHeight() const { return windowHeight; }
int LoadConfig::getSimulationIterations() const { return simulationIterations; }
int LoadConfig::getLoadStarsLimit() const { return loadStarsLimit; }
int LoadConfig::getQuadTreeSearchSize() const { return quadtreeSearchSize; }
bool LoadConfig::getSummaryShowPerProbe() const { return summaryShowPerProbe; }
bool LoadConfig::getSummaryShowFooter() const { return summaryShowFooter; }
int LoadConfig::getprobeIndividualReplicationLimit() const { return probeIndividualReplicationLimit; }
bool LoadConfig::getReplicateOnFirstArrival() const { return replicateOnFirstArrival; }
float LoadConfig::getProbeSearchRadiusParsecs() const { return probeSearchRadiusParsecs; }
float LoadConfig::getProbeSpeedParsecsPerTick() const { return probeSpeedParsecsPerTick; }
float LoadConfig::getViewTiltDegrees() const { return viewTiltDegrees; }
float LoadConfig::getViewDepthParsecs() const { return viewDepthParsecs; }
const std::string &LoadConfig::getStarSpriteStyle() const { return starSpriteStyle; }
const std::string &LoadConfig::getDisplayMode() const { return displayMode; }
bool LoadConfig::getVerticalSync() const { return verticalSync; }
int LoadConfig::getFrameBudgetMillis() const { return frameBudgetMillis; }
float LoadConfig::getCoverageTargetPercent() const { return coverageTargetPercent; }
int LoadConfig::getFrontierStallTicks() const { return frontierStallTicks; }
int LoadConfig::getMaxProbes() const { return maxProbes; }
int LoadConfig::getStarLabelMaxVisible() const { return starLabelMaxVisible; }
float LoadConfig::getZoomMinPixelsPerParsec() const { return zoomMinPixelsPerParsec; }
float LoadConfig::getZoomMaxPixelsPerParsec() const { return zoomMaxPixelsPerParsec; }
bool LoadConfig::getResourcesEnabled() const { return resourcesEnabled; }
float LoadConfig::getSystemResourceScale() const { return systemResourceScale; }
float LoadConfig::getResourceFeatureParsecs() const { return resourceFeatureParsecs; }
int LoadConfig::getResourceSeed() const { return resourceSeed; }
float LoadConfig::getReplicationCostMetals() const { return replicationCostMetals; }
float LoadConfig::getReplicationCostVolatiles() const { return replicationCostVolatiles; }
float LoadConfig::getReplicationCostFissiles() const { return replicationCostFissiles; }
float LoadConfig::getHarvestPerTick() const { return harvestPerTick; }
int LoadConfig::getMaxHarvestTicks() const { return maxHarvestTicks; }
float LoadConfig::getFuelPerParsec() const { return fuelPerParsec; }
float LoadConfig::getFuelSafetyMargin() const { return fuelSafetyMargin; }
float LoadConfig::getChildFuelShare() const { return childFuelShare; }
const std::string &LoadConfig::getTrailColourMode() const { return trailColourMode; }
int LoadConfig::getTrailPalette() const { return trailPalette; }
float LoadConfig::getTrailFadeTicks() const { return trailFadeTicks; }
float LoadConfig::getTrailDensitySaturateAt() const { return trailDensitySaturateAt; }
float LoadConfig::getLineageHueSpread() const { return lineageHueSpread; }
bool LoadConfig::getEvolutionEnabled() const { return evolutionEnabled; }
bool LoadConfig::getNeutralControl() const { return neutralControl; }
float LoadConfig::getMutationStrength() const { return mutationStrength; }
int LoadConfig::getMutationSeed() const { return mutationSeed; }
int LoadConfig::getProbeLabelSize() const { return probeLabelSize; }
bool LoadConfig::getShowStarStalks() const { return showStarStalks; }
bool LoadConfig::getShowStarNames() const { return showStarNames; }
bool LoadConfig::getShowProbeNames() const { return showProbeNames; }
bool LoadConfig::getShowProbeTrails() const { return showProbeTrails; }

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

		readNumber(config, "startingViewRadiusParsecs", startingViewRadiusParsecs);
		if (config.contains("scaleFactor"))
		{
			std::cerr << "Config: 'scaleFactor' is no longer used -- it framed the view by window "
						 "width, which gave wide monitors a shorter view rather than a bigger one. "
						 "Use 'startingViewRadiusParsecs' instead." << std::endl;
		}

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

		readNumber(config, "simulationIterations", simulationIterations);
		readNumber(config, "loadStarsLimit", loadStarsLimit);
		readNumber(config, "quadtreeSearchSize", quadtreeSearchSize);
		readNumber(config, "probeIndividualReplicationLimit", probeIndividualReplicationLimit);
		readNumber(config, "probeSearchRadiusParsecs", probeSearchRadiusParsecs);
		readNumber(config, "probeSpeedParsecsPerTick", probeSpeedParsecsPerTick);
		readNumber(config, "viewTiltDegrees", viewTiltDegrees);
		readNumber(config, "viewDepthParsecs", viewDepthParsecs);
		readNumber(config, "zoomMinPixelsPerParsec", zoomMinPixelsPerParsec);
		readNumber(config, "zoomMaxPixelsPerParsec", zoomMaxPixelsPerParsec);
		readString(config, "starSpriteStyle", starSpriteStyle);
		readString(config, "displayMode", displayMode);
		readBool(config, "verticalSync", verticalSync);
		readNumber(config, "frameBudgetMillis", frameBudgetMillis);
		readNumber(config, "coverageTargetPercent", coverageTargetPercent);
		readNumber(config, "frontierStallTicks", frontierStallTicks);
		readNumber(config, "maxProbes", maxProbes);
		readNumber(config, "starLabelMaxVisible", starLabelMaxVisible);
		readBool(config, "replicateOnFirstArrival", replicateOnFirstArrival);
		readBool(config, "summaryShowPerProbe", summaryShowPerProbe);
		readBool(config, "summaryShowFooter", summaryShowFooter);

		readBool(config, "resourcesEnabled", resourcesEnabled);
		readNumber(config, "systemResourceScale", systemResourceScale);
		readNumber(config, "resourceFeatureParsecs", resourceFeatureParsecs);
		readNumber(config, "resourceSeed", resourceSeed);
		readNumber(config, "replicationCostMetals", replicationCostMetals);
		readNumber(config, "replicationCostVolatiles", replicationCostVolatiles);
		readNumber(config, "replicationCostFissiles", replicationCostFissiles);
		readNumber(config, "harvestPerTick", harvestPerTick);
		readNumber(config, "maxHarvestTicks", maxHarvestTicks);
		readNumber(config, "fuelPerParsec", fuelPerParsec);
		readNumber(config, "fuelSafetyMargin", fuelSafetyMargin);
		readNumber(config, "childFuelShare", childFuelShare);

		readString(config, "trailColourMode", trailColourMode);
		readNumber(config, "trailPalette", trailPalette);
		readNumber(config, "trailFadeTicks", trailFadeTicks);
		readNumber(config, "trailDensitySaturateAt", trailDensitySaturateAt);
		readNumber(config, "lineageHueSpread", lineageHueSpread);
		readBool(config, "evolutionEnabled", evolutionEnabled);
		readBool(config, "neutralControl", neutralControl);
		readNumber(config, "mutationStrength", mutationStrength);
		readNumber(config, "mutationSeed", mutationSeed);
		readNumber(config, "probeLabelSize", probeLabelSize);
		readBool(config, "showStarStalks", showStarStalks);
		readBool(config, "showStarNames", showStarNames);
		readBool(config, "showProbeNames", showProbeNames);
		readBool(config, "showProbeTrails", showProbeTrails);
	}
	catch (json::parse_error &e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << " -- using built-in defaults." << std::endl;
	}
}
