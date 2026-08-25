// LoadConfig.h
#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <string>

class LoadConfig
{
public:
	static LoadConfig &getInstance();

	int getScaleFactor() const;
	int getWindowWidth() const;
	int getWindowHeight() const;
	int getSleepTimeMillis() const;
	int getSimulationIterations() const;
	int getLoadStarsLimit() const;
	int getQuadTreeSearchSize() const;
	bool getSummaryShowPerProbe() const;
	bool getSummaryShowFooter() const;
	int getprobeIndividualReplicationLimit() const;
	int getProbeSearchRadiusPixels() const;
	void loadFromFile();

private:
	LoadConfig();

	// Defaults live here so that a missing config.json, or a missing/invalid key inside
	// it, degrades to a usable value instead of leaving the member indeterminate.
	// loadFromFile() only *overwrites* these -- every one of its validation branches
	// previously logged to cerr and then left the member untouched.
	int scaleFactor = 50;
	int windowWidth = 1280;
	int windowHeight = 720;
	int sleepTimeMillis = 0;
	int simulationIterations = 10000;
	int loadStarsLimit = 10000;
	int quadtreeSearchSize = 128;
	bool summaryShowPerProbe = false;
	bool summaryShowFooter = true;
	int probeIndividualReplicationLimit = 3;
	int probeSearchRadiusPixels = 300;

	// void loadFromFile(const std::string &filename);
	//  Declare copy constructor and assignment operator as private to prevent copying
	LoadConfig(const LoadConfig &) = delete;
	LoadConfig &operator=(const LoadConfig &) = delete;
};

#endif // LOADCONFIG_H