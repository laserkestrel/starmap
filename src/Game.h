// game.h
#ifndef GAME_H
#define GAME_H

#include "LoadConfig.h"
#include "LoadCSVData.h"
#include "Probe.h"
#include "RenderSystem.h"
#include <SFML/Graphics.hpp>
#include "GalaxyQuadTree.h"
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>

class Game
{
public:
	Game(const LoadConfig &config);
	void printProbeVectorContents() const; // Function declaration for printing probeVector contents
	void initializeKeyBindings();
	void run();

private:
	sf::RenderWindow window;
	RenderSystem renderSystem;
	std::vector<Star> galaxyVector;
	std::unordered_map<uint32_t, size_t> starIndexMap; // map star ID -> index in galaxyVector
	// std::vector<Star> galaxyVector2;
	std::vector<Probe> probeVector; // used to keep list of all probe objects so they can be looped through and processed for logic/render.

	void handleEvents();	// will be for reading user input
	void updateGameState(); // will be for running logic of probes and updating star system resources etc.
	void render();			// will render the star objects from galaxyVector
	void renderProbes();	// will render the probe positions on screen with SFML
	void generateSummary() const;
	const LoadConfig &config; // Member variable to hold the LoadConfig object
	int probeSearchRadiusPixels; // editable value that can be changed on the startup screen
	// editable parameters shown on the startup screen (name, value)
	std::vector<std::pair<std::string, std::string>> editableParams;
	int focusedParamIndex;
	sf::Clock caretClock;
	bool caretVisible;
	double simulationTimeInSeconds;
	GalaxyQuadTree theQuadTreeInstance;
	std::unordered_map<sf::Keyboard::Key, std::function<void()>> keyBindings;
};

#endif // GAME_H
