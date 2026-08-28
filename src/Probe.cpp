// Probe.cpp
#include "Probe.h"
#include "Star.h"
#include "DebugLog.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>

namespace
{
	// Seeded once per thread rather than once per call. The old code built a
	// std::random_device and a fresh std::mt19937 -- about 2.5 KB of state --
	// every time a newborn probe moved, and used the non-thread-safe std::rand()
	// for trail colours while probes were being updated in parallel.
	std::mt19937 &rng()
	{
		static thread_local std::mt19937 generator{std::random_device{}()};
		return generator;
	}

	float distance3D(float ax, float ay, float az, float bx, float by, float bz)
	{
		const float dx = ax - bx, dy = ay - by, dz = az - bz;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	// Squared distance, for comparisons where the square root is wasted work.
	float distanceSquared3D(float ax, float ay, float az, float bx, float by, float bz)
	{
		const float dx = ax - bx, dy = ay - by, dz = az - bz;
		return dx * dx + dy * dy + dz * dz;
	}
} // namespace

Probe::Probe(const std::string &probeName, float startX, float startY, float startZ,
			 float speedParsecsPerTick, GalaxyQuadTree &quadTree, const SimSettings &settings)
	: probeName(probeName), x(startX), y(startY), z(startZ),
	  targetX(startX), targetY(startY), targetZ(startZ),
	  speed(speedParsecsPerTick), mode(ProbeMode::Seek),
	  quadTree(&quadTree), settings(&settings)
{
}

std::string Probe::trailToString() const
{
	std::string result;
	for (const auto &visitedSystem : trail)
	{
		result += "Star ID: " + std::to_string(visitedSystem.starID);
		result += " at (" + std::to_string(visitedSystem.coordinates.x) + ", " +
				  std::to_string(visitedSystem.coordinates.y) + ", " +
				  std::to_string(visitedSystem.coordinates.z) + ") pc\n";
	}
	return result;
}

void Probe::setWorldPosition(float newX, float newY, float newZ) { x = newX; y = newY; z = newZ; }
void Probe::setTargetPosition(float newX, float newY, float newZ) { targetX = newX; targetY = newY; targetZ = newZ; }
void Probe::setSpeed(float parsecsPerTick) { speed = parsecsPerTick; }
void Probe::setMode(ProbeMode newMode) { mode = newMode; }
ShutdownReason Probe::getShutdownReason() const { return shutdownReason; }

void Probe::shutdown(ShutdownReason reason)
{
	mode = ProbeMode::Shutdown;
	shutdownReason = reason;
}
void Probe::setNewBorn(bool status) { newBorn = status; }
void Probe::setTargetStar(uint32_t starID) { targetStar = starID; }

void Probe::recordVisit(uint32_t starID, const sf::Vector3f &coordinates)
{
	trail.push_back(VisitedStarSystem{starID, coordinates,
									  settings != nullptr ? settings->currentTick : 0});
	knowledge.learn(starID);
}

void Probe::recordKnown(uint32_t starID)
{
	knowledge.learn(starID);
}

void Probe::forkKnowledgeInto(Probe &child)
{
	knowledge.forkInto(child.knowledge);
}

const std::string &Probe::getProbeName() const { return probeName; }
float Probe::getWorldX() const { return x; }
float Probe::getWorldY() const { return y; }
float Probe::getWorldZ() const { return z; }
float Probe::getSpeed() const { return speed; }
uint32_t Probe::getTargetStar() const { return targetStar; }
ProbeMode Probe::getMode() const { return mode; }
bool Probe::isNewBorn() const { return newBorn; }
float Probe::getTotalDistanceTraveled() const { return totalDistanceTraveled; }
int Probe::getReplicationCount() const { return replicationCount; }
const std::vector<VisitedStarSystem> &Probe::getTrail() const { return trail; }
size_t Probe::getKnownSystemCount() const { return knowledge.knownCount(); }
size_t Probe::getKnowledgeChainDepth() const { return knowledge.chainDepth(); }
sf::Color Probe::getTrailColor() const { return trailColor; }

bool Probe::canAffordReplication() const
{
	if (!settings->resourcesEnabled)
		return true;
	return cargo.covers(settings->replicationCost);
}

Resources Probe::payForReplication()
{
	if (!settings->resourcesEnabled)
		return Resources();

	cargo -= settings->replicationCost;

	// The child leaves with a share of what the parent still holds, which is what
	// stops a newborn stranding on its first hop. It comes out of the parent's tank,
	// so a probe that keeps replicating fuels each child a little worse than the last.
	const float share = std::max(0.0f, std::min(1.0f, settings->childFuelShare));
	const Resources dowry(0.0f, cargo.volatiles * share, 0.0f);
	cargo.volatiles -= dowry.volatiles;
	return dowry;
}

void Probe::move()
{
	if (mode == ProbeMode::Travel)
	{
		const float distanceToTarget = distance3D(targetX, targetY, targetZ, x, y, z);
		const float step = std::min(speed, distanceToTarget);

		// Burn first, then move. A probe that cannot pay for this tick's step is
		// adrift between stars: nothing to mine out there, so this is terminal.
		if (settings->resourcesEnabled)
		{
			const float fuelForStep = step * settings->fuelPerParsec;
			if (cargo.volatiles < fuelForStep)
			{
				cargo.volatiles = 0.0f;
				shutdown(ShutdownReason::StrandedNoFuel);
				return;
			}
			cargo.volatiles -= fuelForStep;
		}

		if (distanceToTarget <= speed)
		{
			// Close enough to land this tick.
			setWorldPosition(targetX, targetY, targetZ);
			totalDistanceTraveled += distanceToTarget;
			recordVisit(targetStar, sf::Vector3f(x, y, z));
			setNewBorn(false);

			if (settings->resourcesEnabled)
			{
				// Mining takes time, so arriving is the start of the work rather
				// than the end of it.
				currentSystem = nullptr; // set by Game, which owns the star vector
				harvestTicksHere = 0;
				setMode(ProbeMode::Harvest);
			}
			else if (replicationCount >= settings->probeIndividualReplicationLimit)
			{
				shutdown(ShutdownReason::ReplicationLimitReached);
			}
			else
			{
				setMode(ProbeMode::Replicate);
			}
		}
		else
		{
			// Step towards the target along the unit vector to it.
			const float inv = 1.0f / distanceToTarget;
			setWorldPosition(x + (targetX - x) * inv * speed,
							 y + (targetY - y) * inv * speed,
							 z + (targetZ - z) * inv * speed);
			totalDistanceTraveled += speed;
		}
	}
	else if (mode == ProbeMode::Harvest)
	{
		// Sitting at a system, extracting. This is the tick cost that turns a
		// wasted journey into a genuinely expensive mistake.
		const float rate = std::max(0.0f, settings->harvestPerTick);
		Resources got;
		if (currentSystem != nullptr)
		{
			got = currentSystem->extract(Resources(rate, rate, rate));
			cargo += got;
			totalMined += got;
		}
		++harvestTicksHere;
		++totalHarvestTicks;

		const bool systemDry = got.empty();
		const bool stayedTooLong = harvestTicksHere >= settings->maxHarvestTicks;
		const bool haveEnough = cargo.covers(settings->replicationCost);

		if (haveEnough || systemDry || stayedTooLong)
		{
			const bool firstArrival = trail.size() <= 1;
			if (firstArrival && !settings->replicateOnFirstArrival)
			{
				setMode(ProbeMode::Seek);
			}
			else if (haveEnough && replicationCount < settings->probeIndividualReplicationLimit)
			{
				setMode(ProbeMode::Replicate);
			}
			else if (replicationCount >= settings->probeIndividualReplicationLimit)
			{
				shutdown(ShutdownReason::ReplicationLimitReached);
			}
			else
			{
				// Could not afford a copy here. Still useful as a scout -- move on
				// and try to make up the shortfall at the next system.
				setMode(ProbeMode::Seek);
			}
		}
	}
	else if (mode == ProbeMode::Replicate)
	{
		// The actual replication happens in Game::updateGameState, which can add
		// to the probe vector safely; this just records that it happened.
		replicationCount++;
		setMode(ProbeMode::Seek);
	}
	else if (mode == ProbeMode::Seek)
	{
		// A newborn used to drift to a random point first, to stop siblings all
		// setting off along the same line. It was guarded on `!trail.empty()`, which
		// is never true for a child -- a child inherits knowledge, not a trail -- so
		// it never once executed. The premise was moot anyway: a parent replicates
		// only once per system, so siblings are never created simultaneously.
		const Star *nearestStar = findNearestUnvisitedStar(quadTree->getRootNode(),
														  settings->probeSearchRadiusParsecs);
		if (nearestStar == nullptr)
		{
			DEBUG_LOG("Probe found no unvisited star within its search radius.");
			shutdown(ShutdownReason::NothingWithinRange);
			return;
		}

		if (settings->resourcesEnabled)
		{
			// Can it actually pay for the trip? A probe that sets off without the
			// fuel to arrive simply dies further from home, which helps nobody.
			const float tripDistance = distance3D(nearestStar->getWorldX(), nearestStar->getWorldY(),
												  nearestStar->getWorldZ(), x, y, z);
			const float fuelNeeded = tripDistance * settings->fuelPerParsec * settings->fuelSafetyMargin;

			if (cargo.volatiles < fuelNeeded)
			{
				// Try to top up where it stands, provided there is anything left here
				// and it has not already given this system its allotted time.
				if (currentSystem != nullptr && !currentSystem->isExhausted() &&
					harvestTicksHere < settings->maxHarvestTicks)
				{
					setMode(ProbeMode::Harvest);
					return;
				}
				shutdown(ShutdownReason::StrandedNoFuel);
				return;
			}
		}

		setTargetPosition(nearestStar->getWorldX(), nearestStar->getWorldY(), nearestStar->getWorldZ());
		setTargetStar(nearestStar->getID());
		setMode(ProbeMode::Travel);
	}
	else if (mode == ProbeMode::Shutdown)
	{
		DEBUG_LOG("Probe has shutdown.");
	}
	else
	{
		std::cout << "Unknown mode.\n";
	}
}

void Probe::setCurrentSystem(const Star *star)
{
	currentSystem = star;
}

void Probe::setBlackTrailColor()
{
	trailColor = sf::Color(0, 0, 0);
}

void Probe::setRandomTrailColor()
{
	const float minBrightness = 100.0f; // keep trails readable against the background
	std::uniform_int_distribution<int> channel(0, 255);

	float red = static_cast<float>(channel(rng()));
	float green = static_cast<float>(channel(rng()));
	float blue = static_cast<float>(channel(rng()));

	const float brightness = 0.299f * red + 0.587f * green + 0.114f * blue;
	if (brightness > 0.0f && brightness < minBrightness)
	{
		// Guarded: an all-zero colour used to divide by zero here, producing inf
		// and then undefined behaviour on the cast back to an integer.
		const float ratio = minBrightness / brightness;
		red = std::min(255.0f, red * ratio);
		green = std::min(255.0f, green * ratio);
		blue = std::min(255.0f, blue * ratio);
	}
	else if (brightness <= 0.0f)
	{
		red = green = blue = minBrightness;
	}

	trailColor = sf::Color(static_cast<sf::Uint8>(red), static_cast<sf::Uint8>(green), static_cast<sf::Uint8>(blue));
}

const Star *Probe::findNearestUnvisitedStar(const GalaxyQuadTreeNode *node, float searchRadiusParsecs) const
{
	if (node == nullptr || node->starVec == nullptr)
	{
		return nullptr;
	}

	// Prune in x/y. A star within 3D radius r has an x/y separation of at most r,
	// so this box can never exclude a star the 3D test would have accepted.
	const float aLeft = node->boundary.left;
	const float aTop = node->boundary.top;
	const float aRight = node->boundary.left + node->boundary.width;
	const float aBottom = node->boundary.top + node->boundary.height;

	if (x + searchRadiusParsecs < aLeft || x - searchRadiusParsecs > aRight ||
		y + searchRadiusParsecs < aTop || y - searchRadiusParsecs > aBottom)
	{
		return nullptr;
	}

	const Star *nearestStar = nullptr;
	float minDistanceSquared = searchRadiusParsecs * searchRadiusParsecs;

	for (const auto &idx : node->starIndices)
	{
		const Star &star = (*node->starVec)[idx];
		// Only this lineage's own knowledge decides. Star::isExplored is a global
		// flag set the moment ANY probe arrives anywhere, so consulting it here gave
		// every probe instantaneous galaxy-wide awareness -- which is not something
		// a probe forty parsecs away could possibly have. It survives purely as an
		// observer's statistic for the summary.
		if (knows(star.getID()))
		{
			continue;
		}
		const float d2 = distanceSquared3D(star.getWorldX(), star.getWorldY(), star.getWorldZ(), x, y, z);
		if (d2 < minDistanceSquared)
		{
			minDistanceSquared = d2;
			nearestStar = &star;
		}
	}

	if (!node->isLeaf)
	{
		for (int i = 0; i < 4; ++i)
		{
			const Star *childNearest = findNearestUnvisitedStar(node->getChild(i), searchRadiusParsecs);
			if (childNearest != nullptr)
			{
				const float d2 = distanceSquared3D(childNearest->getWorldX(), childNearest->getWorldY(),
												   childNearest->getWorldZ(), x, y, z);
				if (d2 < minDistanceSquared)
				{
					minDistanceSquared = d2;
					nearestStar = childNearest;
				}
			}
		}
	}

	return nearestStar;
}
