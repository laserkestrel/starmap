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

	// World-space settings. The probe search radius and speed used to be in
	// screen pixels, which tied how far a probe could see to the window size.
	float getProbeSearchRadiusParsecs() const;
	float getProbeSpeedParsecsPerTick() const;
	// View tilt: 90 = straight down (no stalks), 0 = edge on.
	float getViewTiltDegrees() const;
	// Half-thickness of the slab the view covers, above and below the plane.
	float getViewDepthParsecs() const;
	// Star sprite: "softGlow", "coreHalo", "diffractionSpikes" or "bloomRing".
	const std::string &getStarSpriteStyle() const;
	// Zoom limits, in pixels per parsec.
	float getZoomMinPixelsPerParsec() const;
	float getZoomMaxPixelsPerParsec() const;

	void loadFromFile();

private:
	LoadConfig();

	// Defaults live here so that a missing config.json, or a missing/invalid key
	// inside it, degrades to a usable value instead of leaving the member
	// indeterminate. loadFromFile() only *overwrites* these.
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
	float probeSearchRadiusParsecs = 8.0f;
	float probeSpeedParsecsPerTick = 0.25f;
	float viewTiltDegrees = 75.0f;
	float viewDepthParsecs = 12.0f;
	std::string starSpriteStyle = "coreHalo";
	float zoomMinPixelsPerParsec = 0.5f;
	float zoomMaxPixelsPerParsec = 4000.0f;

	LoadConfig(const LoadConfig &) = delete;
	LoadConfig &operator=(const LoadConfig &) = delete;
};

#endif // LOADCONFIG_H
