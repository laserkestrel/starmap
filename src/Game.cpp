// game.cpp
#include "Game.h"
#include "LoadConfig.h"
#include "LoadCSVData.h"
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
									   config(config),
									   theQuadTreeInstance(sf::FloatRect(0.f, 0.f, config.getWindowWidth(), config.getWindowHeight()), config.getQuadTreeSearchSize())
{
	// Load star systems from JSON file into GalaxyVector
	LoadData dataLoader;
	// galaxyVector = dataLoader.loadStarsFromJson("./content/star_data.json", window, config); // todo - load this from config file

	// Same again using the new CSV loader. (not used at moment, but working in)
	LoadCSVData dataLoader2;
	galaxyVector = dataLoader2.loadStarsFromCsv("./content/hygdata_v40.csv");
	if (!galaxyVector.empty())
	{
		std::cout << "galaxyVector2 vector is populated with " << galaxyVector.size() << " stars." << std::endl;
		for (const Star &star : galaxyVector)
		{
			std::cout << "Star Name: " << star.getName() << " Position (x, y): (" << star.getX() << ", " << star.getY() << ")"
					  << "Color (R, G, B): (" << int(star.getColour().r) << ", " << int(star.getColour().g) << ", " << int(star.getColour().b) << ")" << std::endl;
		}
	}
	else
	{
		std::cout << "galaxyVector vector is empty." << std::endl;
	}

	sf::Vector2u windowSize = window.getSize();

	// Calculate the center coordinates
	int centerX = windowSize.x / 2;
	int centerY = windowSize.y / 2;

	// build a texture of all the stars which can be reused each frame
	// renderSystem.initializeStarsTexture(galaxyVector); //comment out while implementing galaxyVector2 (new csv)
	renderSystem.initializeStarsTexture(galaxyVector);

	// Assume gameBounds represent the entire playable area, and pull quadtree depth from config;
	sf::FloatRect gameBounds(0.f, 0.f, config.getWindowWidth(), config.getWindowHeight());
	int QuadTreeCapacity = config.getQuadTreeSearchSize();

	// Example population of the quadtree in Game.cpp
	for (const auto &star : galaxyVector)
	{
		theQuadTreeInstance.insert(star); // Use 'quadTree' instance to call the insert method
	}
#if defined(_DEBUG)
	// theQuadTreeInstance.debugPrint(); // This will print the structure of the quadtree and the stars in each node EXTREME VERBOSE!
#endif

	// Instantiate a probe class called firstProbe - galaxyVector as argument so data is shared between probe instances.
	// Probe firstProbe("SOL-SOL-AAA", centerX, centerY, 0.0f, galaxyVector, theQuadTreeInstance); // Example coordinates and speed
	Probe firstProbe("SOL-SOL-AAA", centerX, centerY, 0.0f, theQuadTreeInstance); // Example coordinates and speed
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
		sf::sleep(sf::milliseconds(1000));
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
			renderSystem.initializeStarsTexture(galaxyVector);
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
	newProbes.reserve(estimatedReplicationCount);

	// TODO: Can we move logic together into probe class itself?
	// Create new probes based on probesToReplicate

	for (const auto &index : probesToReplicate)
	{
		// Run specific logic when the mode is "Replicate"

		Probe &probe = probeVector[index];

		if (probe.getReplicationCount() > 3)
		{
			probe.setMode(ProbeMode::Shutdown);
		}

		// Create a new replicated probe
		std::string newName = Utilities::probeNamer((probe.getProbeName()), probe.getTargetStar());

		Probe replicatedProbe(newName, probe.getX(), probe.getY(), probe.getSpeed(), theQuadTreeInstance);

		replicatedProbe.setRandomTrailColor();

		// Iterate through visited star systems of the original probe and add to replicated probe
		const std::vector<VisitedStarSystem> &visitedSystems = probe.getVisitedStarSystems();
		for (const auto &visitedSystem : visitedSystems)
		{
			// Set the visitedByProbe to false for this one as the child probe hasn't visited by itself.
			replicatedProbe.addVisitedStarSystem(visitedSystem.systemName, visitedSystem.coordinates, false);
		}

		// TODO: Logic for updating star isExplored property

		newProbes.emplace_back(replicatedProbe);
	}

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

	sf::Sprite starsSprite(renderSystem.getStarsTexture()); // Draw the pre-rendered stars texture
	window.draw(starsSprite);
	renderSystem.renderQuadtree(window, theQuadTreeInstance.getRootNode());

	// render any probes that may exist in probeVector
	for (const auto &probe : probeVector)
	{
		renderSystem.renderProbe(probe);
	}
	renderSystem.calculateAndDisplayFPS();
	window.display();
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