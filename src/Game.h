// game.h
#ifndef GAME_H
#define GAME_H

#include "LoadConfig.h"
#include "LoadCSVData.h"
#include "Probe.h"
#include "RenderSystem.h"
#include <SFML/Graphics.hpp>
#include "GalaxyQuadTree.h"

class Game
{
public:
	Game(const LoadConfig &config);
	void printProbeVectorContents() const; // Function declaration for printing probeVector contents
	void run();

private:
	sf::RenderWindow window;
	RenderSystem renderSystem;
	std::vector<Star> galaxyVector;
	// std::vector<Star> galaxyVector2;
	std::vector<Probe> probeVector; // used to keep list of all probe objects so they can be looped through and processed for logic/render.

	void handleEvents();	// will be for reading user input
	void updateGameState(); // will be for running logic of probes and updating star system resources etc.
	void render();			// will render the star objects from galaxyVector
	void renderProbes();	// will render the probe positions on screen with SFML
	void generateSummary() const;
	const LoadConfig &config; // Member variable to hold the LoadConfig object
	double simulationTimeInSeconds;
	GalaxyQuadTree theQuadTreeInstance;
};

#endif // GAME_H
