// Probe.h
#ifndef PROBE_H
#define PROBE_H

#include "Star.h"
#include "GalaxyQuadTree.h"
#include "SimSettings.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector3.hpp>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

enum class ProbeMode
{
	Travel,
	Replicate,
	Seek,
	Shutdown
};

// A probe's own record of a system it knows about. Coordinates are world
// parsecs, so a trail can be projected through the current view like anything else.
struct VisitedStarSystem
{
	uint32_t starID;
	sf::Vector3f coordinates;
	bool visitedByProbe; // true = this probe went there, false = inherited from its parent
};

class Probe
{
public:
	Probe(const std::string &probeName, float startX, float startY, float startZ,
		  float speedParsecsPerTick, GalaxyQuadTree &quadTree, const SimSettings &settings);

	// NOTE: deliberately no destructor. Declaring one -- even an empty one -- suppresses
	// the implicit move constructor, which forces std::vector<Probe> to *copy* every probe
	// (including its whole visitedStarSystems history) on each reallocation.

	std::string visitedStarSystemsToString() const;

	// Setters
	void setWorldPosition(float x, float y, float z);
	void setTargetPosition(float x, float y, float z);
	void setSpeed(float parsecsPerTick);
	void setMode(ProbeMode mode);
	void setNewBorn(bool status);
	void setTargetStar(uint32_t starID);
	void addVisitedStarSystem(uint32_t starID, const sf::Vector3f &coordinates, bool visitedByProbe);
	void setRandomTrailColor();
	void setBlackTrailColor();

	// Getters
	const std::string &getProbeName() const;
	float getWorldX() const;
	float getWorldY() const;
	float getWorldZ() const;
	float getSpeed() const;
	uint32_t getTargetStar() const;
	ProbeMode getMode() const;
	bool isNewBorn() const;
	float getTotalDistanceTraveled() const;
	int getReplicationCount() const;
	const std::vector<VisitedStarSystem> &getVisitedStarSystems() const;
	sf::Color getTrailColor() const;

	void move();

	// Nearest unvisited star by TRUE 3D distance. The quadtree prunes in x/y
	// only, which is safe -- see the note in GalaxyQuadTreeNode.h.
	const Star *findNearestUnvisitedStar(const GalaxyQuadTreeNode *node, float searchRadiusParsecs) const;

	const GalaxyQuadTreeNode *getCurrentQuadTreeNode() const { return currentQuadTreeNode; }

private:
	bool hasVisited(uint32_t starID) const { return visitedStarIDs.count(starID) != 0; }

	std::string probeName;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float targetX = 0.0f;
	float targetY = 0.0f;
	float targetZ = 0.0f;
	uint32_t targetStar = 0;
	float speed = 0.0f; // parsecs per tick
	ProbeMode mode = ProbeMode::Seek;

	std::vector<VisitedStarSystem> visitedStarSystems; // ordered record, used for trails
	// Same contents keyed for lookup. The search used to scan the vector linearly
	// for every candidate star, which got slower the longer a probe lived.
	std::unordered_set<uint32_t> visitedStarIDs;

	GalaxyQuadTree *quadTree = nullptr;
	const SimSettings *settings = nullptr;
	const GalaxyQuadTreeNode *currentQuadTreeNode = nullptr;
	bool newBorn = true;
	float totalDistanceTraveled = 0.0f;
	int replicationCount = 0;
	sf::Color trailColor = sf::Color::White;
};

#endif // PROBE_H
