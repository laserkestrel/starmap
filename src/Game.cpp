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
#include <sstream>
#include <thread>

namespace
{
	const float PARAM_STEP_RADIUS = 1.0f;
	const int PARAM_STEP_LIMIT = 1;
	// A little slack so a star sitting exactly on the maximum extent is still
	// inside the tree -- sf::FloatRect::contains treats the far edge as outside.
	const float BOUNDS_PADDING_PARSECS = 1.0f;

	// Camera feel. Panning is in screen pixels per second so it covers the same
	// visual distance whatever the zoom; the world distance follows from the scale.
	const float PAN_PIXELS_PER_SECOND = 700.0f;
	const float PAN_FAST_MULTIPLIER = 4.0f; // hold Shift
	const float ZOOM_STEP = 1.15f;          // per mouse-wheel notch
} // namespace

Game::Game(const LoadConfig &config)
	: config(config),
	  displayMode(displayModeFromString(config.getDisplayMode())),
	  window(videoModeFor(displayMode, config.getWindowWidth(), config.getWindowHeight()),
			 "Star Map", windowStyleFor(displayMode)),
	  renderSystem(window, projection, starSpriteStyleFromString(config.getStarSpriteStyle()))
{
	settings.probeSearchRadiusParsecs = config.getProbeSearchRadiusParsecs();
	settings.probeSpeedParsecsPerTick = config.getProbeSpeedParsecsPerTick();
	settings.probeIndividualReplicationLimit = config.getprobeIndividualReplicationLimit();

	window.setVerticalSyncEnabled(config.getVerticalSync());
	renderSystem.setLabelMaxVisible(static_cast<size_t>(std::max(0, config.getStarLabelMaxVisible())));
	std::cout << "Display: " << displayModeName(displayMode) << " "
			  << window.getSize().x << "x" << window.getSize().y << std::endl;

	projection.setTiltDegrees(config.getViewTiltDegrees());
	projection.setViewDepthParsecs(config.getViewDepthParsecs());
	projection.setWorldCentre(sf::Vector2f(0.0f, 0.0f)); // start looking at Sol
	// Scale and centre come from the window's real size, which in fullscreen has
	// nothing to do with the configured width and height.
	syncViewportToWindow();
	resetView();

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

	// The first probe starts at Sol, which is the origin of world space.
	Probe firstProbe("SOL-SOL-AAA", 0.0f, 0.0f, 0.0f, settings.probeSpeedParsecsPerTick, *quadTree, settings);
	firstProbe.setMode(ProbeMode::Seek);
	firstProbe.setNewBorn(false);
	firstProbe.setRandomTrailColor();
	firstProbe.recordVisit(0, sf::Vector3f(0.0f, 0.0f, 0.0f));
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
	// The starfield is drawn fresh every frame now, so these are all instant --
	// none of them has to rebuild anything.
	keyBindings[sf::Keyboard::F1] = [this]() { renderSystem.toggleTextLabelsStars(); };
	keyBindings[sf::Keyboard::F2] = [this]() { renderSystem.toggleTextLabelsProbes(); };
	keyBindings[sf::Keyboard::F3] = [this]() { renderSystem.toggleProbeTrails(); };
	keyBindings[sf::Keyboard::F4] = [this]() { renderSystem.toggleStarStalks(); };
	keyBindings[sf::Keyboard::F5] = [this]() {
		const StarSpriteStyle style = renderSystem.cycleSpriteStyle();
		std::cout << "Star sprite: " << starSpriteStyleName(style) << std::endl;
	};
	keyBindings[sf::Keyboard::F12] = [this]() { renderSystem.toggleDebugGraphics(); };
	keyBindings[sf::Keyboard::Home] = [this]() { resetView(); };
	keyBindings[sf::Keyboard::F11] = [this]() {
		applyDisplayMode(displayMode == DisplayMode::Windowed ? DisplayMode::BorderlessFullscreen
															 : DisplayMode::Windowed);
	};
}

void Game::runStartupScreen()
{
	bool started = false;
	sf::Clock startupClock;
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
			else if (event.type == sf::Event::Resized)
			{
				syncViewportToWindow();
			}
			else if (event.type == sf::Event::MouseWheelScrolled)
			{
				const sf::Vector2f anchor(static_cast<float>(event.mouseWheelScroll.x),
										  static_cast<float>(event.mouseWheelScroll.y));
				const float factor = (event.mouseWheelScroll.delta > 0) ? ZOOM_STEP : (1.0f / ZOOM_STEP);
				projection.zoomAbout(anchor, factor,
									 config.getZoomMinPixelsPerParsec(), config.getZoomMaxPixelsPerParsec());
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

		updateCamera(startupClock.restart().asSeconds());

		renderSystem.renderStarfield(galaxyVector, *quadTree);
		renderSystem.renderParameterList(editableParams, focusedParamIndex, caretVisible);
		renderSystem.renderHud(cameraHudText());
		renderSystem.calculateAndDisplayFPS();
		window.display();
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
	const int frameBudgetMillis = std::max(1, config.getFrameBudgetMillis());
	sf::Clock frameClock;
	int iteration = 0;

	// The simulation and the display used to run in lockstep -- one tick, one frame,
	// forever. That coupling was accidental, and it meant a tick taking 130 ms left
	// the window redrawing seven times a second, so the whole program looked hung.
	//
	// Now each frame spends up to frameBudgetMillis running ticks and then draws
	// regardless. Early on, when a tick is microseconds, that is hundreds of ticks
	// per frame and the simulation runs far faster than it used to. Later it falls
	// to one tick per frame and the simulation slows down -- which it should -- while
	// input and drawing keep going.
	//
	// Honest limit: a frame still cannot be shorter than a single tick, so once one
	// tick alone exceeds the budget the display is bounded by it. Going beyond that
	// needs the simulation on its own thread.
	RunEndReason endReason = RunEndReason::StillRunning;
	while (endReason == RunEndReason::StillRunning && window.isOpen())
	{
		const sf::Time frameElapsed = frameClock.restart();

		handleEvents();
		updateCamera(frameElapsed.asSeconds());

		sf::Clock tickBudget;
		int ticksThisFrame = 0;
		do
		{
			updateGameState();
			++iteration;
			++ticksThisFrame;

			endReason = checkForEnd();
			if (endReason == RunEndReason::StillRunning && iteration >= simulationIterations)
			{
				endReason = RunEndReason::IterationLimit;
			}
		} while (endReason == RunEndReason::StillRunning && window.isOpen() &&
				 tickBudget.getElapsedTime().asMilliseconds() < frameBudgetMillis);

		lastTicksPerFrame = ticksThisFrame;
		render();
	}
	if (!window.isOpen() && endReason == RunEndReason::StillRunning)
	{
		endReason = RunEndReason::Abandoned;
	}
	metrics.endReason = endReason;

	const auto simulationEndTime = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> simulationDuration = simulationEndTime - simulationStartTime;
	simulationTimeInSeconds = simulationDuration.count();
	finaliseMetrics();
	showDebrief();
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
		else if (event.type == sf::Event::Resized)
		{
			// Without this the contents stretch instead of revealing more map.
			syncViewportToWindow();
		}
		else if (event.type == sf::Event::MouseWheelScrolled)
		{
			// Zoom about the cursor, so whatever is under the pointer stays put.
			const sf::Vector2f anchor(static_cast<float>(event.mouseWheelScroll.x),
									  static_cast<float>(event.mouseWheelScroll.y));
			const float factor = (event.mouseWheelScroll.delta > 0) ? ZOOM_STEP : (1.0f / ZOOM_STEP);
			projection.zoomAbout(anchor, factor,
								 config.getZoomMinPixelsPerParsec(), config.getZoomMaxPixelsPerParsec());
		}
	}
}

void Game::updateCamera(float deltaSeconds)
{
	if (!window.hasFocus())
	{
		return;
	}

	float dx = 0.0f, dy = 0.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dx -= 1.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dx += 1.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    dy -= 1.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dy += 1.0f;
	if (dx == 0.0f && dy == 0.0f)
	{
		return;
	}

	float speed = PAN_PIXELS_PER_SECOND * deltaSeconds;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
	{
		speed *= PAN_FAST_MULTIPLIER;
	}
	projection.panByPixels(dx * speed, dy * speed);
}

void Game::resetView()
{
	// Fit a sphere of this radius around Sol. A point R parsecs away can be pushed
	// at most R*ppc from the centre in either screen axis, so the shorter side of
	// the window is what has to accommodate it.
	const float radius = std::max(0.001f, config.getStartingViewRadiusParsecs());
	const float shorterSide = static_cast<float>(std::min(window.getSize().x, window.getSize().y));
	projection.setPixelsPerParsec(shorterSide / (2.0f * radius));
	projection.setWorldCentre(sf::Vector2f(0.0f, 0.0f));
}

// Re-derives what depends on the window's size: a 1-unit-per-pixel view, and the
// screen point the projection maps world origin to. Deliberately does NOT touch
// the zoom, so resizing the window reveals more map rather than yanking the zoom
// back to the configured default mid-session.
void Game::syncViewportToWindow()
{
	const sf::Vector2u size = window.getSize();
	window.setView(sf::View(sf::FloatRect(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y))));
	projection.setScreenCentre(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
}

void Game::applyDisplayMode(DisplayMode mode)
{
	// Keep the camera where it is across the switch -- only the framing changes.
	const sf::Vector2f keptCentre = projection.getWorldCentre();
	const float keptScale = projection.getPixelsPerParsec();

	displayMode = mode;
	window.create(videoModeFor(displayMode, config.getWindowWidth(), config.getWindowHeight()),
				  "Star Map", windowStyleFor(displayMode));
	window.setVerticalSyncEnabled(config.getVerticalSync());

	// Recreating the window can invalidate GL-backed resources.
	renderSystem.reloadGraphicsResources();

	syncViewportToWindow();
	projection.setWorldCentre(keptCentre);
	projection.setPixelsPerParsec(keptScale);
	(void)0;

	std::cout << "Display: " << displayModeName(displayMode) << " "
			  << window.getSize().x << "x" << window.getSize().y << std::endl;
}

std::string Game::cameraHudText() const
{
	const float ppc = projection.getPixelsPerParsec();
	const float across = static_cast<float>(window.getSize().x) / ppc;
	const sf::Vector2f c = projection.getWorldCentre();

	std::ostringstream ss;
	ss.setf(std::ios::fixed);
	ss.precision(1);
	ss << "view " << across << " pc across   centre " << c.x << ", " << c.y << " pc"
	   << "   stars drawn " << renderSystem.getLastVisibleStarCount()
	   << "   probes " << probeVector.size()
	   << "   ticks/frame " << lastTicksPerFrame
	   << "   explored " << metrics.uniqueSystems
	   << "   [arrows pan, shift = faster, wheel = zoom, Home = reset, F5 = sprite]";
	return ss.str();
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
			probe.shutdown(ShutdownReason::ReplicationLimitReached);
			continue;
		}

		const std::string replicationLocationName = Utilities::getStarNameFromID(probe.getTargetStar());
		const std::string newName = Utilities::probeNamer(probe.getProbeName(), replicationLocationName);

		Probe replicatedProbe(newName, probe.getWorldX(), probe.getWorldY(), probe.getWorldZ(),
							  settings.probeSpeedParsecsPerTick, *quadTree, settings);
		replicatedProbe.setRandomTrailColor();

		// The child inherits its parent's knowledge, shared by pointer rather than
		// copied. Copying it was O(everything the ancestry ever saw) per replication,
		// which is what made memory outrun the probe count.
		probe.forkKnowledgeInto(replicatedProbe);

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
			replicatedProbe.recordKnown(parentNextTarget->getID());
		}

		newProbes.push_back(std::move(replicatedProbe));
		++metrics.probesBuilt;
	}

	for (auto &newProbe : newProbes)
	{
		probeVector.push_back(std::move(newProbe));
	}

	// --- parallel probe updates -------------------------------------------------
	std::vector<size_t> oldTrailLengths;
	oldTrailLengths.reserve(probeVector.size());
	for (const auto &probe : probeVector)
	{
		oldTrailLengths.push_back(probe.getTrail().size());
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

	// Mark newly visited stars explored, and gather the run's metrics off the same
	// walk. isExplored is bookkeeping for the observer only -- no probe reads it.
	for (size_t i = 0; i < probeVector.size() && i < oldTrailLengths.size(); ++i)
	{
		const auto &trail = probeVector[i].getTrail();
		for (size_t j = oldTrailLengths[i]; j < trail.size(); ++j)
		{
			++metrics.arrivals;
			auto it = starIndexMap.find(trail[j].starID);
			if (it == starIndexMap.end())
				continue;

			Star &star = galaxyVector[it->second];
			if (star.tryMarkExplored())
			{
				// First time anyone has reached it. Anything else was a wasted trip.
				++metrics.uniqueSystems;
				const double distanceFromSol = std::sqrt(
					static_cast<double>(star.getWorldX()) * star.getWorldX() +
					static_cast<double>(star.getWorldY()) * star.getWorldY() +
					static_cast<double>(star.getWorldZ()) * star.getWorldZ());
				if (distanceFromSol > metrics.frontierParsecs)
				{
					metrics.frontierParsecs = distanceFromSol;
					metrics.lastFrontierAdvanceTick = metrics.ticks;
				}
			}
		}
	}

	// Recount the living. Cheap next to the move phase, and it keeps the
	// end-of-run check O(1).
	liveProbeCount = 0;
	for (const auto &probe : probeVector)
	{
		if (probe.getMode() != ProbeMode::Shutdown)
			++liveProbeCount;
	}

	++metrics.ticks;
	metrics.peakPopulation = std::max(metrics.peakPopulation, probeVector.size());
	const double coverage = metrics.coveragePercent();
	if (metrics.ticksTo25 < 0 && coverage >= 25.0) metrics.ticksTo25 = metrics.ticks;
	if (metrics.ticksTo50 < 0 && coverage >= 50.0) metrics.ticksTo50 = metrics.ticks;
	if (metrics.ticksTo75 < 0 && coverage >= 75.0) metrics.ticksTo75 = metrics.ticks;
}

void Game::render()
{
	renderSystem.renderStarfield(galaxyVector, *quadTree);
	renderSystem.renderQuadtree(window, quadTree->getRootNode());
	renderSystem.renderProbes(probeVector);
	renderSystem.renderHud(cameraHudText());
	renderSystem.calculateAndDisplayFPS();
	window.display();
}

RunEndReason Game::checkForEnd() const
{
	// Counted as probes shut down rather than rescanned: at a million probes a
	// full scan every tick is itself a meaningful cost.
	if (liveProbeCount == 0)
		return RunEndReason::AllProbesStopped;

	const int cap = config.getMaxProbes();
	if (cap > 0 && probeVector.size() >= static_cast<size_t>(cap))
		return RunEndReason::PopulationCap;

	if (metrics.coveragePercent() >= config.getCoverageTargetPercent())
		return RunEndReason::CoverageReached;

	const int stallLimit = config.getFrontierStallTicks();
	if (stallLimit > 0 && metrics.ticks - metrics.lastFrontierAdvanceTick > stallLimit)
		return RunEndReason::FrontierStalled;

	return RunEndReason::StillRunning;
}

void Game::finaliseMetrics()
{
	metrics.catalogueSize = galaxyVector.size();
	metrics.distanceFlownParsecs = 0.0;
	metrics.probesAlive = 0;
	metrics.stoppedAtReplicationLimit = 0;
	metrics.stoppedWithNothingInRange = 0;

	for (const auto &probe : probeVector)
	{
		metrics.distanceFlownParsecs += probe.getTotalDistanceTraveled();
		if (probe.getMode() != ProbeMode::Shutdown)
		{
			++metrics.probesAlive;
			continue;
		}
		switch (probe.getShutdownReason())
		{
		case ShutdownReason::ReplicationLimitReached: ++metrics.stoppedAtReplicationLimit; break;
		case ShutdownReason::NothingWithinRange:      ++metrics.stoppedWithNothingInRange; break;
		default: break;
		}
	}

	metrics.wallClockSeconds = simulationTimeInSeconds;

	// Still printed to the console as well as drawn on screen: headless runs, CI
	// and piping to a file all need it, and it is where you look when something
	// like a missing content folder has gone wrong.
	if (config.getSummaryShowPerProbe())
	{
		for (const auto &probe : probeVector)
		{
			std::cout << "- Probe [" << probe.getProbeName() << "] travelled ["
					  << probe.getTotalDistanceTraveled() << " pc], replicated ["
					  << probe.getReplicationCount() << "] times, visited ["
					  << probe.getTrail().size() << "] systems" << std::endl;
		}
	}
	if (config.getSummaryShowFooter())
	{
		metrics.printToConsole();
	}
}

void Game::showDebrief()
{
	// The map stays live underneath, so you can still pan and zoom around the
	// result while reading it.
	sf::Clock viewClock;
	while (window.isOpen())
	{
		handleEvents();
		updateCamera(viewClock.restart().asSeconds());

		renderSystem.renderStarfield(galaxyVector, *quadTree);
		renderSystem.renderQuadtree(window, quadTree->getRootNode());
		renderSystem.renderProbes(probeVector);
		renderSystem.renderDebrief(metrics);
		renderSystem.renderHud(cameraHudText());
		renderSystem.calculateAndDisplayFPS();
		window.display();
	}
}
