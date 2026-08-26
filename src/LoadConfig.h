// LoadConfig.h
#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <string>

class LoadConfig
{
public:
	static LoadConfig &getInstance();

	// Starting zoom, expressed as the radius around Sol guaranteed to be in frame.
	// Defining it as a radius rather than "parsecs across the width" makes the
	// opening view the same neighbourhood on any resolution or aspect ratio -- a
	// wider monitor simply shows more to either side.
	float getStartingViewRadiusParsecs() const;
	int getWindowWidth() const;
	int getWindowHeight() const;
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
	// "windowed", "borderlessFullscreen" or "exclusiveFullscreen".
	const std::string &getDisplayMode() const;
	// Cap the loop to the monitor's refresh rate. Off by default, because the
	// render loop is also the simulation loop -- turning it on slows the sim down
	// to your refresh rate as well as smoothing the display.
	bool getVerticalSync() const;
	// How much of each frame may be spent running simulation ticks. The renderer
	// gets whatever is left, so the display stays responsive instead of being
	// dragged along at whatever rate the simulation happens to manage.
	int getFrameBudgetMillis() const;
	// A run ends when any of these is met, so it stops on its own and reports why
	// rather than running until you press Escape.
	float getCoverageTargetPercent() const;
	int getFrontierStallTicks() const;
	// Safety valve. Without a cost to replication the fleet grows without bound --
	// a 600-star galaxy reached 1.9 million probes in testing -- so a run needs a
	// ceiling until something physical (resources, attrition) bounds it instead.
	int getMaxProbes() const;
	// How many star labels to draw at once. The brightest are chosen, so names
	// thin out as you zoom out instead of vanishing all at once.
	int getStarLabelMaxVisible() const;
	// Zoom limits, in pixels per parsec.
	float getZoomMinPixelsPerParsec() const;
	float getZoomMaxPixelsPerParsec() const;

	void loadFromFile();

private:
	LoadConfig();

	// Defaults live here so that a missing config.json, or a missing/invalid key
	// inside it, degrades to a usable value instead of leaving the member
	// indeterminate. loadFromFile() only *overwrites* these.
	float startingViewRadiusParsecs = 8.0f;
	int windowWidth = 1280;
	int windowHeight = 720;
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
	std::string displayMode = "borderlessFullscreen";
	int starLabelMaxVisible = 120;
	bool verticalSync = false;
	int frameBudgetMillis = 8;
	float coverageTargetPercent = 100.0f;
	int frontierStallTicks = 5000;
	int maxProbes = 250000;
	float zoomMinPixelsPerParsec = 0.5f;
	float zoomMaxPixelsPerParsec = 4000.0f;

	LoadConfig(const LoadConfig &) = delete;
	LoadConfig &operator=(const LoadConfig &) = delete;
};

#endif // LOADCONFIG_H
