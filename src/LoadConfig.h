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
	// May a probe replicate at the first system it reaches, or must it establish
	// itself there first? A real lever on how fast the fleet grows.
	bool getReplicateOnFirstArrival() const;

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

	// --- the resource economy ---------------------------------------------------
	// Replication used to be free, which is why every run ended the same way: at
	// the population cap. These give it a price.
	bool getResourcesEnabled() const;
	// Units of each resource in an averagely rich system.
	float getSystemResourceScale() const;
	// How big a rich or poor region is, in parsecs. Small values give a speckled
	// galaxy where every neighbourhood averages the same; large values give broad
	// lodes and deserts worth expanding towards or away from.
	float getResourceFeatureParsecs() const;
	// Fixes the galaxy's resource geography. The same seed gives the same galaxy on
	// every run, which is what makes two runs with different parameters comparable.
	int getResourceSeed() const;
	// What one copy costs to build.
	float getReplicationCostMetals() const;
	float getReplicationCostVolatiles() const;
	float getReplicationCostFissiles() const;
	// Units of each resource a probe can take from a system per tick. Lower means
	// longer stays and a slower, more deliberate expansion.
	float getHarvestPerTick() const;
	// Give up on a system after this many ticks even if it still holds something.
	int getMaxHarvestTicks() const;
	// Volatiles burnt per parsec flown. This is what makes distance dangerous.
	float getFuelPerParsec() const;
	// A probe will not depart unless it holds this multiple of the fuel the trip
	// needs. Below 1.0 it will take trips it cannot finish.
	float getFuelSafetyMargin() const;
	// Share of the parent's remaining volatiles handed to a new probe.
	float getChildFuelShare() const;

	// --- trail appearance -------------------------------------------------------
	// "recency", "density" or "perProbe". F6 cycles them while running.
	const std::string &getTrailColourMode() const;
	// Index into the palette table in TrailStyle.cpp. F7 cycles them.
	int getTrailPalette() const;
	// Ticks for a trail to cool from white to dark in recency mode.
	float getTrailFadeTicks() const;
	// Arrivals at which a system is fully saturated in density mode.
	float getTrailDensitySaturateAt() const;
	// How much of the colour wheel the whole family tree spans, 0..1.
	float getLineageHueSpread() const;

	// Point size for probe name labels.
	int getProbeLabelSize() const;

	// Which overlays start switched on. All four are also F keys at runtime.
	bool getShowStarStalks() const;
	bool getShowStarNames() const;
	bool getShowProbeNames() const;
	bool getShowProbeTrails() const;

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
	bool replicateOnFirstArrival = false;
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

	// Resource economy defaults. Tuned so an averagely rich system funds roughly
	// one copy: expansion is possible everywhere but comfortable nowhere.
	bool resourcesEnabled = true;
	float systemResourceScale = 100.0f;
	float resourceFeatureParsecs = 6.0f;
	int resourceSeed = 20260828;
	float replicationCostMetals = 55.0f;
	float replicationCostVolatiles = 35.0f;
	float replicationCostFissiles = 8.0f;
	float harvestPerTick = 6.0f;
	int maxHarvestTicks = 60;
	float fuelPerParsec = 1.5f;
	float fuelSafetyMargin = 1.25f;
	float childFuelShare = 0.35f;
	std::string trailColourMode = "recency";
	int trailPalette = 0;
	float trailFadeTicks = 600.0f;
	float trailDensitySaturateAt = 60.0f;
	float lineageHueSpread = 1.0f;
	int probeLabelSize = 15;
	bool showStarStalks = true;
	bool showStarNames = false;
	bool showProbeNames = false;
	bool showProbeTrails = false;

	LoadConfig(const LoadConfig &) = delete;
	LoadConfig &operator=(const LoadConfig &) = delete;
};

#endif // LOADCONFIG_H
