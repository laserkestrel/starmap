// Game.cpp
#include "Game.h"
#include "DebugLog.h"
#include "LoadCSVData.h"
#include "Utilities.h"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>

namespace
{
	const float PARAM_STEP_RADIUS = 1.0f;
	const int PARAM_STEP_LIMIT = 1;
	// A little slack so a star sitting exactly on the maximum extent is still
	// inside the tree -- sf::FloatRect::contains treats the far edge as outside.
	const float BOUNDS_PADDING_PARSECS = 1.0f;
} // namespace

Game::Game(const LoadConfig &config)
	: config(config),
	  window(sf::VideoMode(config.getWindowWidth(), config.getWindowHeight()), "Star Map"),
	  renderSystem(window, projection)
{
	settings.probeSearchRadiusParsecs = config.getProbeSearchRadiusParsecs();
	settings.probeSpeedParsecsPerTick = config.getProbeSpeedParsecsPerTick();
	settings.probeIndividualReplicationLimit = config.getprobeIndividualReplicationLimit();

	// scaleFactor is how many parsecs span the window's width.
	const float scaleFactor = std::max(1.0f, static_cast<float>(config.getScaleFactor()));
	projection.setPixelsPerParsec(static_cast<float>(config.getWindowWidth()) / scaleFactor);
	projection.setTiltDegrees(config.getViewTiltDegrees());
	projection.setViewDepthParsecs(config.getViewDepthParsecs());
	projection.setCentre(sf::Vector2f(config.getWindowWidth() / 2.0f, config.getWindowHeight() / 2.0f));

	LoadCSVData dataLoader;
	galaxyVector = dataLoader.loadStarsFromCsv("./content/hygdata_v40.csv", config);
	if (galaxyVector.empty())
	{
		std::cerr << "No stars loaded -- check ./content/hygdata_v40.csv" << std::endl;
	}
	else
	{
		std::cout << "Loaded " << galaxyVector.size() << " stars." << std::endl;
	}

	Utilities::populateStarData(galaxyVector);

	starIndexMap.reserve(galaxyVector.size());
	for (size_t i = 0; i < galaxyVector.size(); ++i)
	{
		starIndexMap[galaxyVector[i].getID()] = i;
	}

	// Quadtree bounded by the data, not the viewport.
	const sf::FloatRect bounds = computeCatalogueBounds();
	quadTree = std::make_unique<GalaxyQuadTree>(bounds, config.getQuadTreeSearchSize());
	quadTree->setStarVector(&galaxyVector);

	size_t rejected = 0;
	for (size_t i = 0; i < galaxyVector.size(); ++i)
	{
		if (!quadTree->insert(i))
		{
			++rejected;
		}
	}
	std::cout << "Quadtree spans " << bounds.width << " x " << bounds.height
			  << " pc; " << (galaxyVector.size() - rejected) << " of " << galaxyVector.size()
			  << " stars indexed";
	if (rejected > 0)
	{
		std::cout << " (" << rejected << " REJECTED -- this should be zero)";
	}
	std::cout << "." << std::endl;

	renderSystem.initializeStarsTexture(galaxyVector);

	// The first probe starts at Sol, which is the origin of world space.
	Probe firstProbe("SOL-SOL-AAA", 0.0f, 0.0f, 0.0f, settings.probeSpeedParsecsPerTick, *quadTree, settings);
	firstProbe.setMode(ProbeMode::Seek);
	firstProbe.setNewBorn(false);
	firstProbe.setRandomTrailColor();
	firstProbe.addVisitedStarSystem(0, sf::Vector3f(0.0f, 0.0f, 0.0f), true);
	probeVector.push_back(std::move(firstProbe));

	editableParams.emplace_back("probeSearchRadiusParsecs", std::to_string(settings.probeSearchRadiusParsecs));
	editableParams.emplace_back("probeIndividualReplicationLimit", std::to_string(settings.probeIndividualReplicationLimit));
}

sf::FloatRect Game::computeCatalogueBounds() const
{
	if (galaxyVector.empty())
	{
		return sf::FloatRect(-1.0f, -1.0f, 2.0f, 2.0f);
	}

	float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max(), maxY = std::numeric_limits<float>::lowest();
	for (const auto &star : galaxyVector)
	{
		minX = std::min(minX, star.getWorldX());
		maxX = std::max(maxX, star.getWorldX());
		minY = std::min(minY, star.getWorldY());
		maxY = std::max(maxY, star.getWorldY());
	}
	minX -= BOUNDS_PADDING_PARSECS;
	minY -= BOUNDS_PADDING_PARSECS;
	maxX += BOUNDS_PADDING_PARSECS;
	maxY += BOUNDS_PADDING_PARSECS;
	return sf::FloatRect(minX, minY, maxX - minX, maxY - minY);
}

void Game::initializeKeyBindings()
{
	keyBindings[sf::Keyboard::Escape] = [this]() { window.close(); };
	// Only the toggles that change the starfield rebuild its texture. F2 flips
	// probe labels, which has nothing to do with the stars.
	keyBindings[sf::Keyboard::F1] = [this]() {
		renderSystem.toggleTextLabelsStars();
		renderSystem.initializeStarsTexture(galaxyVector);
	};
	keyBindings[sf::Keyboard::F2] = [this]() { renderSystem.toggleTextLabelsProbes(); };
	keyBindings[sf::Keyboard::F3] = [this]() { renderSystem.toggleProbeTrails(); };
	keyBindings[sf::Keyboard::F4] = [this]() {
		renderSystem.toggleStarStalks();
		renderSystem.initializeStarsTexture(galaxyVector);
	};
	keyBindings[sf::Keyboard::F12] = [this]() { renderSystem.toggleDebugGraphics(); };
}

void Game::runStartupScreen()
{
	bool started = false;
	while (!started && window.isOpen())
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
				const int count = static_cast<int>(editableParams.size());
				if (event.key.code == sf::Keyboard::Enter)
				{
					started = true;
				}
				else if (event.key.code == sf::Keyboard::Tab && count > 0)
				{
					focusedParamIndex = event.key.shift ? (focusedParamIndex - 1 + count) % count
														: (focusedParamIndex + 1) % count;
				}
				else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down)
				{
					const float direction = (event.key.code == sf::Keyboard::Up) ? 1.0f : -1.0f;
					try
					{
						if (focusedParamIndex == 0)
						{
							float v = std::stof(editableParams[0].second) + direction * PARAM_STEP_RADIUS;
							editableParams[0].second = std::to_string(std::max(0.1f, v));
						}
						else
						{
							int v = std::stoi(editableParams[1].second) + static_cast<int>(direction) * PARAM_STEP_LIMIT;
							editableParams[1].second = std::to_string(std::max(0, v));
						}
					}
					catch (...) {}
				}
				else
				{
					auto it = keyBindings.find(event.key.code);
					if (it != keyBindings.end())
					{
						it->second();
					}
				}
			}
			else if (event.type == sf::Event::TextEntered)
			{
				const unsigned int uni = event.text.unicode;
				std::string &value = editableParams[focusedParamIndex].second;
				if ((uni >= '0' && uni <= '9') || uni == '.')
				{
					value.push_back(static_cast<char>(uni));
				}
				else if (uni == 8 && !value.empty()) // backspace
				{
					value.pop_back();
				}
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				started = true;
			}
		}

		if (caretClock.getElapsedTime().asMilliseconds() > 500)
		{
			caretVisible = !caretVisible;
			caretClock.restart();
		}

		window.clear();
		window.draw(sf::Sprite(renderSystem.getStarsTexture()));
		renderSystem.renderParameterList(editableParams, focusedParamIndex, caretVisible);
		renderSystem.calculateAndDisplayFPS();
		window.display();

		sf::sleep(sf::milliseconds(50));
	}

	// Apply BOTH edited values. Previously only editableParams[0] was read back,
	// and even that went into a Game member the probes never looked at.
	try { settings.probeSearchRadiusParsecs = std::max(0.1f, std::stof(editableParams[0].second)); }
	catch (...) {}
	try { settings.probeIndividualReplicationLimit = std::max(0, std::stoi(editableParams[1].second)); }
	catch (...) {}

	std::cout << "Starting with search radius " << settings.probeSearchRadiusParsecs
			  << " pc, replication limit " << settings.probeIndividualReplicationLimit << "." << std::endl;
}

void Game::run()
{
	initializeKeyBindings();
	runStartupScreen();

	const auto simulationStartTime = std::chrono::high_resolution_clock::now();
	const int simulationIterations = config.getSimulationIterations();
	const int sleepTimeMillis = config.getSleepTimeMillis();
	sf::Clock clock;
	int iteration = 0;

	while (iteration < simulationIterations && window.isOpen())
	{
		const sf::Time elapsed = clock.restart();

		handleEvents();
		updateGameState();
		render();
		++iteration;

		const sf::Time sleepTime = sf::milliseconds(sleepTimeMillis);
		if (elapsed < sleepTime)
		{
			sf::sleep(sleepTime - elapsed);
		}
	}

	const auto simulationEndTime = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> simulationDuration = simulationEndTime - simulationStartTime;
	simulationTimeInSeconds = simulationDuration.count();
	std::cout << "Simulation took " << simulationTimeInSeconds << " seconds." << std::endl;
	generateSummary();

	while (window.isOpen())
	{
		handleEvents();
		render();
		sf::sleep(sf::milliseconds(200));
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
	}
}

void Game::updateGameState()
{
	// --- replication -----------------------------------------------------------
	std::vector<size_t> probesToReplicate;
	for (size_t i = 0; i < probeVector.size(); ++i)
	{
		if (probeVector[i].getMode() == ProbeMode::Replicate)
		{
			probesToReplicate.push_back(i);
		}
	}

	std::vector<Probe> newProbes;
	newProbes.reserve(probesToReplicate.size());

	for (const auto &index : probesToReplicate)
	{
		Probe &probe = probeVector[index];

		if (probe.getReplicationCount() >= settings.probeIndividualReplicationLimit)
		{
			probe.setMode(ProbeMode::Shutdown);
			continue;
		}

		const std::string replicationLocationName = Utilities::getStarNameFromID(probe.getTargetStar());
		const std::string newName = Utilities::probeNamer(probe.getProbeName(), replicationLocationName);

		Probe replicatedProbe(newName, probe.getWorldX(), probe.getWorldY(), probe.getWorldZ(),
							  settings.probeSpeedParsecsPerTick, *quadTree, settings);
		replicatedProbe.setRandomTrailColor();

		// The child inherits its parent's knowledge, but not the credit for it.
		for (const auto &visitedSystem : probe.getVisitedStarSystems())
		{
			replicatedProbe.addVisitedStarSystem(visitedSystem.starID, visitedSystem.coordinates, false);
		}

		// Tell the child where its parent is headed next, so the two do not both
		// set off for the same star.
		const GalaxyQuadTreeNode *searchRoot = probe.getCurrentQuadTreeNode();
		if (searchRoot == nullptr)
		{
			searchRoot = quadTree->getRootNode();
		}
		const Star *parentNextTarget = probe.findNearestUnvisitedStar(searchRoot, settings.probeSearchRadiusParsecs);
		if (parentNextTarget != nullptr)
		{
			replicatedProbe.addVisitedStarSystem(parentNextTarget->getID(),
												 sf::Vector3f(parentNextTarget->getWorldX(),
															  parentNextTarget->getWorldY(),
															  parentNextTarget->getWorldZ()),
												 false);
		}

		newProbes.push_back(std::move(replicatedProbe));
	}

	for (auto &newProbe : newProbes)
	{
		probeVector.push_back(std::move(newProbe));
	}

	// --- parallel probe updates -------------------------------------------------
	std::vector<size_t> oldVisitedCounts;
	oldVisitedCounts.reserve(probeVector.size());
	for (const auto &probe : probeVector)
	{
		oldVisitedCounts.push_back(probe.getVisitedStarSystems().size());
	}

	const unsigned int threadCount = std::max(1u, std::thread::hardware_concurrency());
	const size_t totalProbes = probeVector.size();
	const size_t chunk = (totalProbes + threadCount - 1) / threadCount;

	std::vector<std::thread> workers;
	workers.reserve(threadCount);
	for (unsigned int t = 0; t < threadCount; ++t)
	{
		const size_t start = t * chunk;
		const size_t end = std::min(start + chunk, totalProbes);
		if (start >= end)
			continue;

		workers.emplace_back([this, start, end]() {
			for (size_t i = start; i < end; ++i)
			{
				probeVector[i].move();
			}
		});
	}
	for (auto &w : workers)
	{
		if (w.joinable())
			w.join();
	}

	// Mark newly visited stars explored.
	for (size_t i = 0; i < probeVector.size() && i < oldVisitedCounts.size(); ++i)
	{
		const auto &visited = probeVector[i].getVisitedStarSystems();
		for (size_t j = oldVisitedCounts[i]; j < visited.size(); ++j)
		{
			auto it = starIndexMap.find(visited[j].starID);
			if (it != starIndexMap.end())
			{
				galaxyVector[it->second].tryMarkExplored();
			}
		}
	}
}

void Game::render()
{
	window.clear();
	window.draw(sf::Sprite(renderSystem.getStarsTexture()));
	renderSystem.renderQuadtree(window, quadTree->getRootNode());
	renderSystem.renderProbes(probeVector);
	renderSystem.calculateAndDisplayFPS();
	window.display();
}

void Game::generateSummary() const
{
	std::cout << "-----------------" << '\n'
			  << "Begin Summary: " << '\n'
			  << "-----------------" << '\n';

	if (config.getSummaryShowPerProbe())
	{
		for (const auto &probe : probeVector)
		{
			if (probe.getTotalDistanceTraveled() > 0 && probe.getReplicationCount() > 0)
			{
				std::cout << "- Probe [" << probe.getProbeName() << "] travelled ["
						  << probe.getTotalDistanceTraveled() << " pc], replicated ["
						  << probe.getReplicationCount() << "] times, visiting ";
				for (const auto &visitedSystem : probe.getVisitedStarSystems())
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

	size_t totalStarsVisitedByProbes = 0;
	for (const auto &star : galaxyVector)
	{
		if (star.getIsExplored())
		{
			++totalStarsVisitedByProbes;
		}
	}

	if (config.getSummaryShowFooter())
	{
		const size_t probeCount = probeVector.size();
		// Guarded: this used to divide by probeCount and elapsed time without
		// checking either.
		const double denominator = simulationTimeInSeconds * static_cast<double>(probeCount);
		const double starsPerProbeSecond = (denominator > 0.0)
											   ? static_cast<double>(totalStarsVisitedByProbes) / denominator
											   : 0.0;
		const double coverage = galaxyVector.empty()
									? 0.0
									: 100.0 * static_cast<double>(totalStarsVisitedByProbes) / static_cast<double>(galaxyVector.size());

		std::cout << "Simulation Summary:" << '\n'
				  << "Simulation limited to [" << config.getSimulationIterations() << "] epochs" << '\n'
				  << "Total number of stars: " << galaxyVector.size() << '\n'
				  << "Total number of probes: " << probeCount << '\n'
				  << "Stars explored: " << totalStarsVisitedByProbes << " (" << coverage << "% of catalogue)" << '\n'
				  << "Total Simulation Time: " << simulationTimeInSeconds << " seconds." << '\n'
				  << "Stars per probe-second: " << starsPerProbeSecond << '\n'
				  << "-----------------" << std::endl;
	}
}
