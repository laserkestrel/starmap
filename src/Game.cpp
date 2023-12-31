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
	// Load star systems from CSV file into GalaxyVector
	LoadCSVData dataLoader2;
	galaxyVector = dataLoader2.loadStarsFromCsv("./content/hygdata_v40.csv", window, config);
	if (!galaxyVector.empty())
	{
		std::cout << "galaxyVector2 vector is populated with " << galaxyVector.size() << " stars." << std::endl;
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

	// Create a mapping table of star names to their ID values. Used for passing to probe namer.
	Utilities::populateStarData(galaxyVector);

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
	// create a mapping of starID values to star names.

	// Instantiate a probe class called firstProbe - galaxyVector as argument so data is shared between probe instances.
	// Probe firstProbe("SOL-SOL-AAA", centerX, centerY, 0.0f, galaxyVector, theQuadTreeInstance); // Example coordinates and speed
	Probe firstProbe("SOL-SOL-AAA", centerX, centerY, 0.0f, theQuadTreeInstance); // Example coordinates and speed
	firstProbe.setMode(ProbeMode::Seek);
	firstProbe.setNewBorn(false);
	firstProbe.setRandomTrailColor();
	sf::Vector2f SolCoordinates(centerX, centerY); // Replace these values with actual coordinates
	firstProbe.addVisitedStarSystem(0, SolCoordinates, true);
	firstProbe.setSpeed(1); // make sure starter system is set
	firstProbe.move();		// currently running the actual logic of the probe from its class.
	// and add it to the probeVector (a list of all probes in simulation)
	probeVector.push_back(firstProbe);
}

void Game::initializeKeyBindings()
{
	keyBindings[sf::Keyboard::Escape] = [this]()
	{ window.close(); };
	keyBindings[sf::Keyboard::F1] = [this]()
	{ renderSystem.toggleTextLabelsStars(); renderSystem.initializeStarsTexture(galaxyVector); };
	keyBindings[sf::Keyboard::F2] = [this]()
	{ renderSystem.toggleTextLabelsProbes(); renderSystem.initializeStarsTexture(galaxyVector); };
	keyBindings[sf::Keyboard::F3] = [this]()
	{ renderSystem.toggleProbeTrails(); };
	keyBindings[sf::Keyboard::F12] = [this]()
	{ renderSystem.toggleDebugGraphics(); };
}

void Game::run()
{
	initializeKeyBindings();
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
		render();
		sf::sleep(sf::milliseconds(200));
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
			auto it = keyBindings.find(event.key.code);
			if (it != keyBindings.end())
			{
				it->second();
			}
		}
		else if (event.type == sf::Event::MouseWheelScrolled)
		{
			if (event.mouseWheelScroll.delta > 0)
			{
				// Zoom in
				// view.zoom(0.8f);
				std::cout << "Do some Zoom In" << '\n';
			}
			else
			{
				// Zoom out
				std::cout << "Do some Zoom Out" << '\n';
				// view.zoom(1.2f);
			}
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

		// int probereplimit;
		// probereplimit = config.getprobeIndividualReplicationLimit();
		// so if replication count is 0, and our limit is 0 (do not replicate EQUAL TO OR GTR THAN)
		// if replication count is 0 and our limit is 1 - replicate
		// if replication count is 1 and our limit is 1 - do not replicate (EQUAL TO OR GTR THAN)

		if (probe.getReplicationCount() >= config.getprobeIndividualReplicationLimit())
		{
			probe.setMode(ProbeMode::Shutdown);
		}
		else
		{ // if probe hasnt reached its replication limit, do some replicating. shouldnt we be doing this before a probe goes into replication mode?!
			// Create a new replicated probe
			// must be using targetStar name to generate the child probe name string.
			// first arg is used as parent name, second string as replication location.

			// Need to convert current location ID to string name. use utility class.
			uint32_t replicationLocationID;															   // declare new varaible
			replicationLocationID = probe.getTargetStar();											   // get the probes current target ID
			std::string replicationLocationName = Utilities::getStarNameFromID(replicationLocationID); // pass target ID into lookup utility, returns string of system name.

			std::string newName = Utilities::probeNamer((probe.getProbeName()), replicationLocationName);
			Probe replicatedProbe(newName, probe.getX(), probe.getY(), probe.getSpeed(), theQuadTreeInstance);

			replicatedProbe.setRandomTrailColor();

			// Iterate through visited star systems of the original probe and add to replicated probe
			const std::vector<VisitedStarSystem> &visitedSystems = probe.getVisitedStarSystems();
			for (const auto &visitedSystem : visitedSystems)
			{
				// Set the visitedByProbe to false for this one as the child probe hasn't visited by itself.
				replicatedProbe.addVisitedStarSystem(visitedSystem.starID, visitedSystem.coordinates, false);
			}

			// TODO: Get next target Star for current probe, pass this as a visted system to child.
			// step 1 - to get next target, we need the findNearestUnvisitedStarInQuadTree (FNUSIQT) function
			// step 2- FNUSIQT needs the probes current quadtree location, and a search radius. (hard code, but setup TODO into config.800 is value from other part doing same.)
			const GalaxyQuadTreeNode *parentProbeCurrentQuadTreeLocation = probe.getCurrentQuadTreeNode();
			// step 3 - we dont have implementation for current quadtree location! - we do now.
			// step 4-  we also dont have anything to set the quadtree location.
			const Star *parentProbeNextTarget = probe.findNearestUnvisitedStarInQuadTree(parentProbeCurrentQuadTreeLocation, config.getProbeSearchRadiusPixels());
			if (parentProbeNextTarget != nullptr)
			{
				// step 5 - need to convert the xy into avector object
				replicatedProbe.addVisitedStarSystem(parentProbeNextTarget->getID(), sf::Vector2f(parentProbeNextTarget->getX(), parentProbeNextTarget->getY()), false);

				// debug here
			}
			else
			{
				// Handle the case where no nearest unvisited star was found
			}

			// TODO: Logic for updating star isExplored property

			newProbes.emplace_back(replicatedProbe);
		}
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
				std::cout << "- Probe Name: [" << probe.getProbeName() << "] Traveled [" << probe.getTotalDistanceTraveled() << "], replicated [" << probe.getReplicationCount() << "] times,"
						  << "visiting ";

				const std::vector<VisitedStarSystem> &visitedSystems = probe.getVisitedStarSystems();
				for (const auto &visitedSystem : visitedSystems)
				{
					if (visitedSystem.visitedByProbe)
					{
						std::cout << "[" << visitedSystem.starID << "];";
					}
				}
				std::cout << std::endl;
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
				  // TODO - FIX and workout what a perfect score is on this. aparantly min number for 7 stars is 3 probes.
				  << "Efficiency Ratio: " << efficiencyScore << '\n'
				  << "Efficiency Percent: " << efficiencyPercentage << '\n'
				  << "-----------------" << std::endl;
	}
}