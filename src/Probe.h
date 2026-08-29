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
	Harvest,
	Replicate,
	Seek,
	Shutdown
};

// Why a probe stopped. Diagnostic, and worth reading as a set: lots of
// ReplicationLimitReached means the replication limit is holding exploration back,
// lots of NothingWithinRange means the search radius is, and lots of
// StrandedNoFuel means the galaxy is too poor for the range being asked of it --
// probes reaching for stars they cannot afford to arrive at.
enum class ShutdownReason
{
	StillRunning,
	ReplicationLimitReached,
	NothingWithinRange,
	StrandedNoFuel
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
	// The tick this arrival happened on. Four bytes per stop, and it is what lets the
	// renderer fade a trail by age instead of drawing the whole history at one
	// brightness -- which is why the expansion front was impossible to pick out.
	long long arrivalTick = 0;
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
	// --- identity and lineage ---------------------------------------------------
	// A probe's path through the family tree: one letter per generation, saying which
	// child it was. Empty for the root. A child extends it by one letter, so every
	// name is unique without a central registry, the length is the generation, and a
	// shared prefix means shared ancestry.
	void setLineagePath(const std::string &path) { lineagePath = path; }
	const std::string &getLineagePath() const { return lineagePath; }
	int getGeneration() const { return static_cast<int>(lineagePath.size()); }
	void setProbeName(const std::string &name) { probeName = name; }
	// Short form for drawing on the map; the full name stays the identity.
	void setDisplayName(const std::string &name) { displayName = name; }
	const std::string &getDisplayName() const { return displayName.empty() ? probeName : displayName; }

	// The root probe owns an arc of the colour wheel and each child is given a slice
	// of its parent's arc, so a whole subtree occupies one contiguous band of hue and
	// how far apart two probes look is how far apart they are in the tree.
	//
	// Stepping the hue a fixed amount per generation was tried first and does not
	// work: the drift accumulates, wraps the wheel every few generations, and distant
	// strangers end up the same colour as close relatives.
	void setLineageArc(float arcStart, float arcWidth);
	float getLineageHue() const { return lineageArcStart; }
	// Hands `child` its slice. childIndex is how many copies this probe has already
	// made; maxChildren is how many it is allowed, which sets how finely the arc splits.
	void deriveLineageInto(Probe &child, int childIndex, int maxChildren) const;
	void setBlackTrailColor();

	// --- resources --------------------------------------------------------------
	const Resources &getCargo() const { return cargo; }
	void setCargo(const Resources &r) { cargo = r; }
	int getHarvestTicks() const { return totalHarvestTicks; }
	const Resources &getTotalMined() const { return totalMined; }
	// Can this probe afford a copy right now?
	bool canAffordReplication() const;
	// Deducts the build cost and returns the starting cargo to hand the child.
	// Called by Game at the moment the child is actually created, so a probe that
	// is refused a copy (say by the population cap) is not charged for one.
	Resources payForReplication();
	// Game owns the star vector, so it is Game that tells a probe which system it
	// has arrived at. Probe only ever reads through the pointer.
	void setCurrentSystem(const Star *star);
	const Star *getCurrentSystem() const { return currentSystem; }

	// Where this probe was built. A newborn's trail is empty until it reaches its
	// first system, so without this its opening leg could not be drawn at all -- the
	// child appeared to detach from its parent and drift off unattached.
	sf::Vector3f getBirthPosition() const { return birthPosition; }

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
	sf::Color getLineageColour() const;

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
	sf::Color lineageColour = sf::Color::White;
	// This probe's slice of the colour wheel, in turns. Its own hue is the start of
	// the arc; its children divide up the rest.
	float lineageArcStart = 0.0f;
	float lineageArcWidth = 1.0f;
	std::string lineagePath; // "" for the root; length is the generation
	std::string displayName;

	// What it is carrying, what it has dug up in total, and the system it is
	// currently sitting in. The star pointer is into Game's galaxy vector, which
	// outlives every probe and is never reallocated during a run.
	Resources cargo;
	Resources totalMined;
	sf::Vector3f birthPosition;
	const Star *currentSystem = nullptr;
	int harvestTicksHere = 0;
	int totalHarvestTicks = 0;
};

#endif // PROBE_H
