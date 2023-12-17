// LoadConfig.h
#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <string>

class LoadConfig
{
public:
	static LoadConfig& getInstance(const std::string& filename);

	int getScaleFactor() const;
	int getWindowWidth() const;
	int getWindowHeight() const;
	int getSleepTimeMillis() const;
	int getSimulationIterations() const;
	unsigned int getWorldSeed() const;

private:
	LoadConfig(const std::string& filename);

	int scaleFactor;
	int windowWidth;
	int windowHeight;
	int sleepTimeMillis;
	int simulationIterations = 0;
	unsigned int worldSeed = 0;

	void loadFromFile(const std::string& filename);
	// Declare copy constructor and assignment operator as private to prevent copying
	LoadConfig(const LoadConfig&) = delete;
	LoadConfig& operator=(const LoadConfig&) = delete;
};

#endif // LOADCONFIG_H