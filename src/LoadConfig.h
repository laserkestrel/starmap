// LoadConfig.h
#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <string>

class LoadConfig
{
public:
	static LoadConfig &getInstance(const std::string &filename);

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

private:
	LoadConfig(const std::string &filename);

	int scaleFactor;
	int windowWidth;
	int windowHeight;
	int sleepTimeMillis;
	int simulationIterations = 0;
	int loadStarsLimit = 999999;
	unsigned int worldSeed = 0;
	int quadtreeSearchSize;
	bool summaryShowPerProbe;
	bool summaryShowFooter;

	void loadFromFile(const std::string &filename);
	// Declare copy constructor and assignment operator as private to prevent copying
	LoadConfig(const LoadConfig &) = delete;
	LoadConfig &operator=(const LoadConfig &) = delete;
};

#endif // LOADCONFIG_H