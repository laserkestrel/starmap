// game.cpp
#include "Game.h"
#include "LoadConfig.h"
#include "Probe.h"
#include "RenderSystem.h"
#include "Utilities.h"
#include "GalaxyQuadTree.h"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <iostream>

Game::Game(const LoadConfig &config) :

									   window(sf::VideoMode(config.getWindowWidth(), config.getWindowHeight()), "Star Map"),
									   renderSystem(window),
									   config(config)
{
	// Load star systems from JSON file into GalaxyVector
	LoadData dataLoader;
	galaxyVector = dataLoader.loadStarsFromJson("./content/star_data.json", window, config); // todo - load this from config file
	sf::Vector2u windowSize = window.getSize();

	// build a texture of all the stars which can be reused each frame
	renderSystem.initializeStarsTexture(galaxyVector);

	// Example initialization in Game.cpp
	// Assume gameBounds represent the entire playable area, and pull quadtree depth from config;
	sf::FloatRect gameBounds(0.f, 0.f, config.getWindowWidth(), config.getWindowHeight());
	int QuadTreeCapacity = config.getQuadTreeSearchSize();
	// Initialize the GalaxyQuadTree with the game boundaries and a suitable capacity
	GalaxyQuadTree quadTree(gameBounds, QuadTreeCapacity);

	// Example population of the quadtree in Game.cpp
	for (const auto &star : galaxyVector)
	{
		quadTree.insert(star); // Use 'quadTree' instance to call the insert method
	}

	// Calculate the center coordinates
	int centerX = windowSize.x / 2;
	int centerY = windowSize.y / 2;
	// Instantiate a probe class called firstProbe - galaxyVector as argument so data is shared between probe instances.
	Probe firstProbe("SOL-SOL-AAA", centerX, centerY, 0.0f, galaxyVector); // Example coordinates and speed
	firstProbe.setMode(ProbeMode::Seek);
	firstProbe.setNewBorn(false);
	firstProbe.setRandomTrailColor();
	sf::Vector2f SolCoordinates(centerX, centerY); // Replace these values with actual coordinates
	firstProbe.addVisitedStarSystem("Sol", SolCoordinates, true);
	firstProbe.setSpeed(1); // make sure starter system is set
	firstProbe.move();		// currently running the actual logic of the probe from its class.
	// and add it to the probeVector (a list of all probes in simulation)
	probeVector.push_back(firstProbe);
}

void Game::run()
{
	// Start measuring time before entering the simulation loop
	auto simulationStartTime = std::chrono::high_resolution_clock::now();
	int simulationIterations = config.getSimulationIterations(); // Use config received in the constructor
	int sleepTimeMillis = config.getSleepTimeMillis();			 // Retrieve sleep time from the received config
	sf::Clock clock;
	int iteration = 0;

	// Run the simulation loop for the specified number of iterations
	while (iteration < simulationIterations && window.isOpen())
	{
		sf::Time elapsed = clock.restart();

		handleEvents();
		updateGameState();
		render();

		++iteration;

		sf::Time sleepTime = sf::milliseconds(sleepTimeMillis); // Convert sleepTimeMillis to sf::Time

		// Check if elapsed time is less than sleepTime, then sleep for the remaining time
		if (elapsed < sleepTime)
		{
			sf::sleep(sleepTime - elapsed);
		}
	}
	// Generate summary after simulation iterations
	// Calculate the elapsed time after the simulation completes
	auto simulationEndTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> simulationDuration = simulationEndTime - simulationStartTime;

	double simulationTimeInSeconds = simulationDuration.count();
	this->simulationTimeInSeconds = simulationTimeInSeconds; // Assign value
	std::cout << "Simulation took " << simulationTimeInSeconds << " seconds." << std::endl;
	Game::generateSummary();

	// After the simulation finishes, keep the window open for user interactions
	while (window.isOpen())
	{
		handleEvents();
		// Add other relevant operations for the window here
	}
}

void Game::handleEvents()
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			window.close();
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Escape)
			{
				// generateSummary();
				window.close();
			}
		}
		else if (event.key.code == sf::Keyboard::F2)
		{
			// Toggle text labels visibility
			renderSystem.toggleTextLabels();
		}
		else if (event.key.code == sf::Keyboard::F3)
		{
			// Toggle probe trails visibility
			renderSystem.toggleProbeTrails();
		}
	}
}

void Game::updateGameState()
{
	std::vector<Probe> newProbes;		   // Store new probes to add later
	std::vector<size_t> probesToReplicate; // Store indices of probes to replicate

	// Find probes that need to replicate
	for (size_t i = 0; i < probeVector.size(); ++i)
	{
		if (probeVector[i].getMode() == ProbeMode::Replicate)
		{
			// Store the index of the probe that needs replication
			probesToReplicate.push_back(i);
		}
	}

	// Estimate the required capacity for new probes (adjust as needed)
	size_t estimatedReplicationCount = probesToReplicate.size();
	newProbes.reserve(estimatedReplicationCount); // Missing semicolon here

	// COMMENT HERE TO AVOID REPLICATION
	// Create new probes based on probesToReplicate
	for (const auto &index : probesToReplicate)
	{
// Run specific logic when the mode is "Replicate"
#if defined(_DEBUG)
		std::cout << "Probe is in Replicate mode from the Game loop, and is spawning a new probe instance now\n";
#endif
		const Probe &probe = probeVector[index];

		// Create a new replicated probe
		std::string newName = Utilities::probeNamer((probe.getProbeName()), probe.getTargetStar());
		Probe replicatedProbe(newName, probe.getX(), probe.getY(), probe.getSpeed(), galaxyVector);
		// std::cout << "Replicating probe [" << probe.getProbeName() << "] targetstar or child birthplace is ..." << probe.getTargetStar() << '\n';
		replicatedProbe.setRandomTrailColor();

		// Iterate through visited star systems of the original probe and add to replicated probe
		// IS THIS IN THE CONTEXT OF THE CHILD PROBE, NOT THE PARENT!? -doesnt seem to be.
		const std::vector<VisitedStarSystem> &visitedSystems = probe.getVisitedStarSystems();
		// BUG issue #18 - missing logic to populate the birthplace system first.
		// how do we set this, shouldnt this birthplace be in the visited system of the parent yet? Take the child, add the parents current location to its properties.
		// am I making this hard, cant I just add the parents current location to visitedStarSystem BEFORE replication?
		// so when does visitedSystem get populated?
		// replicatedProbe.addVisitedStarSystem((probe.getTargetStar()),(probe.getX()),true);
		// are these null visited sytems where the probe tries to get nearestStarSystem by radius and there isnt one? - no because they visit somewhere else after.
		for (const auto &visitedSystem : visitedSystems)
		{
#if defined(_DEBUG)
			std::cout << "Adding this system to child probe - [" << visitedSystem.systemName << "]" << '\n';
#endif
			// Set the visitedByProbe to false for this one as the child probe hasn't visited by itself.
			replicatedProbe.addVisitedStarSystem(visitedSystem.systemName, visitedSystem.coordinates, false);
		}

		// TODO: Logic for updating star isExplored property

		newProbes.emplace_back(replicatedProbe);
	}
	// UNCOMMENT HERE TO AVOID REPLICATION

	// Add new probes created during replication mode to the main probe vector
	for (const auto &newProbe : newProbes)
	{
		probeVector.push_back(newProbe);
	}

	// Move all probes after handling replication
	for (auto &probe : probeVector)
	{
		probe.move(); // Execute the movement logic for each probe
	}

	// Add your game logic for updating the state here
}

void Game::render()
{
	window.clear();
	// Draw the pre-rendered stars texture
	sf::Sprite starsSprite(renderSystem.getStarsTexture());
	window.draw(starsSprite);

	// render any probes that may exist in probeVector
	for (const auto &probe : probeVector)
	{
		renderSystem.renderProbe(probe);
	}
	renderSystem.calculateAndDisplayFPS();
	window.display();
	// printProbeVectorContents(); //print some debug stuff
}

void Game::generateSummary() const
{
	// Collect and display header summary statistics here
	std::cout << "-----------------" << '\n'
			  << "Begin Summary: " << '\n'
			  << "-----------------" << '\n';

	if (config.getSummaryShowPerProbe())
	{
		for (const auto &probe : probeVector)
		{
			if (probe.getTotalDistanceTraveled() > 0 && probe.getReplicationCount() > 0)
			{
				std::cout << "- Probe Name: [" << probe.getProbeName() << "] Traveled [" << probe.getTotalDistanceTraveled() << "], replicated [" << probe.getReplicationCount() << "] times, visiting ";

				const std::vector<VisitedStarSystem> &visitedSystems = probe.getVisitedStarSystems();
				for (const auto &visitedSystem : visitedSystems)
				{
					if (visitedSystem.visitedByProbe)
					{
						std::cout << "[" << visitedSystem.systemName << "];";
					}
				}
				std::cout << '\n';
			}
		}
	}
	// Footer statistics if needed (total distance, total replications, etc.)

	int summarySeed = config.getWorldSeed();
	int summaryIterations = config.getSimulationIterations();
	size_t probeCount = probeVector.size();

	size_t totalStarsVisitedByProbes = 0;

	for (const auto &star : galaxyVector)
	{
		if (star.getIsExplored())
		{
			totalStarsVisitedByProbes++;
		}
	}
	double maxPossibleEfficiencyScore = 1;
	double efficiencyScore = totalStarsVisitedByProbes / (simulationTimeInSeconds * probeCount);
	double efficiencyPercentage = (efficiencyScore / maxPossibleEfficiencyScore) * 100.0;

	if (config.getSummaryShowFooter())
	{
		// show summary footer is true
		std::cout << "Simulation Summary:" << '\n'
				  << "World Seed is [" << summarySeed << "] and Simulation limited to [" << summaryIterations << "] epochs" << '\n'
				  << "Total number of stars: " << galaxyVector.size() << '\n'
				  << "Total number of probes: " << probeCount << '\n'
				  << "Total Simulation Time: " << simulationTimeInSeconds << " seconds." << '\n'
				  // need to work out what a perfect score is on this. aparantly min number for 7 stars is 3 probes.
				  << "Efficiency Ratio: " << efficiencyScore << '\n'
				  << "Efficiency Percent: " << efficiencyPercentage << '\n'
				  << "-----------------" << std::endl;
	}
}
