// Probe.cpp
#include "Probe.h"
#include "Star.h"
#include "DebugLog.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
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
	  quadTree(&quadTree), settings(&settings),
	  birthPosition(startX, startY, startZ)
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
sf::Color Probe::getLineageColour() const { return lineageColour; }

const Genome &Probe::behaviour() const
{
	return (settings != nullptr && settings->neutralControl) ? settings->founderGenome : genome;
}

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
	const float share = std::max(0.0f, std::min(1.0f, behaviour().childFuelShare));
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
			else if (replicationCount >= behaviour().replicationLimitInt())
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
		const bool stayedTooLong = harvestTicksHere >= behaviour().harvestPatienceInt();
		const bool haveEnough = cargo.covers(settings->replicationCost);

		if (haveEnough || systemDry || stayedTooLong)
		{
			const bool firstArrival = trail.size() <= 1;
			if (firstArrival && !settings->replicateOnFirstArrival)
			{
				setMode(ProbeMode::Seek);
			}
			else if (haveEnough && replicationCount < behaviour().replicationLimitInt())
			{
				setMode(ProbeMode::Replicate);
			}
			else if (replicationCount >= behaviour().replicationLimitInt())
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
														  behaviour().searchRadiusParsecs);
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
					harvestTicksHere < behaviour().harvestPatienceInt())
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
	lineageColour = sf::Color(0, 0, 0);
}

namespace
{
	// Hue in turns (0..1) to RGB, at fixed saturation and value. Fixing those two is
	// deliberate: a label has to stay readable against a black starfield, and letting
	// the generator wander into dark or washed-out colours would trade legibility for
	// variety nobody asked for.
	sf::Color hueToColour(float hue, float saturation, float value)
	{
		hue -= std::floor(hue); // wrap into 0..1
		const float h = hue * 6.0f;
		const int sector = static_cast<int>(h) % 6;
		const float f = h - std::floor(h);
		const float p = value * (1.0f - saturation);
		const float q = value * (1.0f - saturation * f);
		const float t = value * (1.0f - saturation * (1.0f - f));

		float r = 0.0f, g = 0.0f, b = 0.0f;
		switch (sector)
		{
		case 0: r = value; g = t;     b = p;     break;
		case 1: r = q;     g = value; b = p;     break;
		case 2: r = p;     g = value; b = t;     break;
		case 3: r = p;     g = q;     b = value; break;
		case 4: r = t;     g = p;     b = value; break;
		default: r = value; g = p;    b = q;     break;
		}
		return sf::Color(static_cast<sf::Uint8>(r * 255.0f),
						 static_cast<sf::Uint8>(g * 255.0f),
						 static_cast<sf::Uint8>(b * 255.0f));
	}
} // namespace

void Probe::setLineageArc(float arcStart, float arcWidth)
{
	lineageArcStart = arcStart - std::floor(arcStart);
	lineageArcWidth = std::max(0.0f, arcWidth);
	// The probe's own colour is the MIDDLE of its arc, not the edge. Taking the edge
	// put every parent on the boundary it shares with a sibling subtree, so a parent
	// looked identical to a cousin it was not closely related to.
	const float centre = lineageArcStart + lineageArcWidth * 0.5f;
	lineageColour = hueToColour(centre - std::floor(centre), 0.72f, 1.0f);
}

void Probe::deriveLineageInto(Probe &child, int childIndex, int maxChildren) const
{
	// Children divide up the WHOLE of their parent's arc, one slice each in birth
	// order. Sibling subtrees never overlap, so every descendant of a given probe
	// stays inside that probe's band of the wheel and colour distance tracks distance
	// through the family tree.
	//
	// An earlier version reserved the first slice for the parent itself and split only
	// what was left. That sounds tidier and is measurably worse: the reserved slice is
	// never subdivided, so with a replication limit of 3 the whole 0-90 degree wedge --
	// every red, orange and yellow -- became unreachable. Simulated over 3,000 probes
	// it produced no hue at all between 0.02 and 0.22 of the wheel.
	const int slices = std::max(1, maxChildren);
	const float sliceWidth = lineageArcWidth / static_cast<float>(slices);
	const int slot = std::min(std::max(0, childIndex), slices - 1);
	child.setLineageArc(lineageArcStart + sliceWidth * static_cast<float>(slot), sliceWidth);
}

// Nearest unvisited star by TRUE 3D distance. The quadtree prunes in x/y only,
// which is safe -- see the note in GalaxyQuadTreeNode.h.
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
