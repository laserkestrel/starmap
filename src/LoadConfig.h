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
	unsigned int getWorldSeed() const;
	int getQuadTreeSearchSize() const;
	bool getSummaryShowPerProbe() const;
	bool getSummaryShowFooter() const;
	int getprobeIndividualReplicationLimit() const;
	int getProbeSearchRadiusPixels() const;
	void loadFromFile();

private:
	LoadConfig();

	int scaleFactor;
	int windowWidth;
	int windowHeight;
	int sleepTimeMillis;
	int simulationIterations;
	int loadStarsLimit;
	unsigned int worldSeed;
	int quadtreeSearchSize;
	bool summaryShowPerProbe;
	bool summaryShowFooter;
	int probeIndividualReplicationLimit;
	int probeSearchRadiusPixels;

	// void loadFromFile(const std::string &filename);
	//  Declare copy constructor and assignment operator as private to prevent copying
	LoadConfig(const LoadConfig &) = delete;
	LoadConfig &operator=(const LoadConfig &) = delete;
};

#endif // LOADCONFIG_H