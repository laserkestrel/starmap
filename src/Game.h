// Game.h
#ifndef GAME_H
#define GAME_H

#include "DisplayMode.h"
#include "GalaxyQuadTree.h"
#include "LoadConfig.h"
#include "Probe.h"
#include "Projection.h"
#include "HighScores.h"
#include "RunMetrics.h"
#include "SetupUI.h"
#include "RenderSystem.h"
#include "SimSettings.h"
#include "Star.h"
#include "StarSprite.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Game
{
public:
	Game(const LoadConfig &config);
	void initializeKeyBindings();
	void run();

private:
	void runStartupScreen();
	// Re-reads the catalogue at a new size and rebuilds everything that depends on
	// it. Needed because the setup screen can change how many stars are loaded.
	void reloadGalaxy(int starLimit);
	void seedFirstProbe();
	std::string reachSummary() const;
	// Arrow-key panning, polled each frame so held keys move smoothly.
	void updateCamera(float deltaSeconds);
	void resetView();
	// Recreates the window in `mode` and re-derives everything that depends on
	// its size. Safe to call at any time.
	void applyDisplayMode(DisplayMode mode);
	void syncViewportToWindow();
	std::string cameraHudText() const;
	void handleEvents();
	void updateGameState();
	void render();
	void finaliseMetrics();
	RunEndReason checkForEnd() const;
	// What the player chose at the end of a run.
	enum class DebriefChoice { RunAgain, Quit };
	DebriefChoice showDebrief(int newRank);
	void runSimulation();
	// World-space bounding box of the loaded catalogue, used as the quadtree's
	// boundary. Previously this was the window rectangle, which is why almost
	// every star fell outside the tree and no probe could ever reach it.
	sf::FloatRect computeCatalogueBounds() const;

	const LoadConfig &config;
	SimSettings settings;   // mutable copy the startup screen edits and probes read
	Projection projection;  // the one world -> screen transform

	DisplayMode displayMode = DisplayMode::BorderlessFullscreen;
	sf::RenderWindow window;
	RenderSystem renderSystem;

	std::vector<Star> galaxyVector;
	std::unordered_map<uint32_t, size_t> starIndexMap; // star ID -> index in galaxyVector
	std::vector<Probe> probeVector;

	// Built after the catalogue is loaded, since its bounds come from the data.
	std::unique_ptr<GalaxyQuadTree> quadTree;

	SetupUI setupUI;
	int loadedStarLimit = 0;
	// Richness the current galaxy was built with. Changing it on the setup screen
	// means re-deriving every star's stocks, so it reloads like a size change does.
	float loadedRichness = -1.0f;
	// Mutation happens in the single-threaded replication phase, so one generator is
	// enough and a fixed seed keeps two runs of the same settings comparable.
	std::mt19937 mutationRng{20260829u};
	int activeMaxProbes = 250000;
	double simulationTimeInSeconds = 0.0;
	int lastTicksPerFrame = 0;
	RunMetrics metrics;
	HighScores highScores;
	bool debriefVisible = true;
	size_t liveProbeCount = 1;

	std::unordered_map<sf::Keyboard::Key, std::function<void()>> keyBindings;
};

#endif // GAME_H
