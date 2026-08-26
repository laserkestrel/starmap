// Probe.h
#ifndef PROBE_H
#define PROBE_H

#include "Star.h"
#include "GalaxyQuadTree.h"
#include "Knowledge.h"
#include "SimSettings.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector3.hpp>
#include <cstdint>
#include <string>
#include <vector>

enum class ProbeMode
{
	Travel,
	Replicate,
	Seek,
	Shutdown
};

// Why a probe stopped. The split between these two is diagnostic: lots of the
// first means the replication limit is what is holding exploration back, lots of
// the second means the search radius is.
enum class ShutdownReason
{
	StillRunning,
	ReplicationLimitReached,
	NothingWithinRange
};

// One stop on a probe's own journey. Coordinates are world parsecs, so a trail can
// be projected through the current view like anything else.
//
// This is only what THIS probe visited -- inherited knowledge lives in Knowledge and
// is shared rather than copied. The two were previously conflated in one vector,
// which is why memory grew faster than the probe count.
struct VisitedStarSystem
{
	uint32_t starID;
	sf::Vector3f coordinates;
};

class Probe
{
public:
	Probe(const std::string &probeName, float startX, float startY, float startZ,
		  float speedParsecsPerTick, GalaxyQuadTree &quadTree, const SimSettings &settings);

	// NOTE: deliberately no destructor. Declaring one -- even an empty one -- suppresses
	// the implicit move constructor, which forces std::vector<Probe> to *copy* every probe
	// (including its whole visitedStarSystems history) on each reallocation.

	std::string trailToString() const;

	// Setters
	void setWorldPosition(float x, float y, float z);
	void setTargetPosition(float x, float y, float z);
	void setSpeed(float parsecsPerTick);
	void setMode(ProbeMode mode);
	void shutdown(ShutdownReason reason);
	void setNewBorn(bool status);
	void setTargetStar(uint32_t starID);
	// Arrived somewhere: goes on the trail and into this probe's knowledge.
	void recordVisit(uint32_t starID, const sf::Vector3f &coordinates);
	// Knows about somewhere without having been there (e.g. a parent's next target).
	void recordKnown(uint32_t starID);
	// Hands the child everything known so far, shared rather than copied.
	void forkKnowledgeInto(Probe &child);
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
	ShutdownReason getShutdownReason() const;
	bool isNewBorn() const;
	float getTotalDistanceTraveled() const;
	int getReplicationCount() const;
	const std::vector<VisitedStarSystem> &getTrail() const;
	size_t getKnownSystemCount() const;
	size_t getKnowledgeChainDepth() const;
	sf::Color getTrailColor() const;

	void move();

	// Nearest unvisited star by TRUE 3D distance. The quadtree prunes in x/y
	// only, which is safe -- see the note in GalaxyQuadTreeNode.h.
	const Star *findNearestUnvisitedStar(const GalaxyQuadTreeNode *node, float searchRadiusParsecs) const;

	const GalaxyQuadTreeNode *getCurrentQuadTreeNode() const { return currentQuadTreeNode; }

private:
	bool knows(uint32_t starID) const { return knowledge.knows(starID); }

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
	ShutdownReason shutdownReason = ShutdownReason::StillRunning;

	std::vector<VisitedStarSystem> trail; // only where this probe itself went
	Knowledge knowledge;                  // what it and its ancestors know, shared

	GalaxyQuadTree *quadTree = nullptr;
	const SimSettings *settings = nullptr;
	const GalaxyQuadTreeNode *currentQuadTreeNode = nullptr;
	bool newBorn = true;
	float totalDistanceTraveled = 0.0f;
	int replicationCount = 0;
	sf::Color trailColor = sf::Color::White;
};

#endif // PROBE_H
