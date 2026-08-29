// RunMetrics.h
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include "Resources.h"
#include "Genome.h"
#include <vector>

// Why a run stopped. A simulation that only ends when you press Escape cannot be
// compared with another one, because the numbers describe an arbitrary moment.
enum class RunEndReason
{
	StillRunning,
	AllProbesStopped,   // nothing left that can move
	CoverageReached,    // hit the target share of the catalogue
	FrontierStalled,    // no new ground for a long time
	IterationLimit,     // ran out of configured ticks
	PopulationCap,      // hit the safety limit on fleet size
	Abandoned           // the window was closed
};

const char *runEndReasonText(RunEndReason reason);

// Everything worth knowing about a run.
//
// Deliberately measured per TICK and as ratios, never per wall-clock second. A
// metric that divides by elapsed time is measuring the computer it ran on, which
// is why the old "stars per probe-second" figure could never compare two runs.
struct RunMetrics
{
	// --- raw counters, accumulated as the run goes ---
	long long ticks = 0;
	long long arrivals = 0;        // every time any probe reached a system
	long long uniqueSystems = 0;   // systems reached for the first time by anyone
	long long probesBuilt = 1;     // including the one you start with
	double frontierParsecs = 0.0;  // furthest system reached, from Sol
	size_t peakPopulation = 1;
	long long lastFrontierAdvanceTick = 0;

	// --- filled in at the end ---
	double distanceFlownParsecs = 0.0;
	size_t catalogueSize = 0;
	size_t probesAlive = 0;
	size_t stoppedAtReplicationLimit = 0;
	size_t stoppedWithNothingInRange = 0;
	size_t stoppedStranded = 0;     // ran dry -- the new way to fail
	double wallClockSeconds = 0.0; // reported for interest only, never used in a metric
	RunEndReason endReason = RunEndReason::StillRunning;

	// --- the resource economy ---
	bool resourcesEnabled = false;
	Resources totalMined;            // everything dug out of every system
	Resources resourcesSpentOnProbes; // what all that building cost
	long long harvestTicks = 0;      // probe-ticks spent sitting still, mining
	size_t systemsExhausted = 0;     // systems stripped to nothing
	double peakFleetShareMining = 0.0; // busiest moment of the mining economy

	// Ticks taken to reach each quarter of the catalogue; -1 if never reached.
	long long ticksTo25 = -1, ticksTo50 = -1, ticksTo75 = -1;

	// --- evolution ---
	// Population-average traits per generation, taken from every probe ever born.
	// Bucketing by generation gives the whole trajectory for free: probes are never
	// removed from the fleet vector when they shut down, so at the end of a run the
	// vector IS the complete family record.
	struct GenerationTraits
	{
		size_t population = 0;
		Genome mean;
	};
	bool evolutionEnabled = false;
	float mutationStrength = 0.0f;
	Genome founderGenome;
	std::vector<GenerationTraits> generations; // index is the generation number

	// The last generation with a meaningful population, so the report compares the
	// founder against a generation that actually had members rather than a tail of
	// one or two stragglers.
	int lastPopulousGeneration(size_t minimumPopulation = 5) const;

	// --- derived ---
	double coveragePercent() const;
	// 1.0 means no journey was wasted. 0.5 means half of all arrivals were to a
	// system some other lineage had already reached. This is the number the
	// per-lineage knowledge model exists to produce.
	double efficiency() const;
	long long wastedJourneys() const;
	double parsecsPerDiscovery() const;
	double probesPerDiscovery() const;
	double expansionRatePerThousandTicks() const;
	// Systems reached per 1000 units of everything mined. The resource-economy
	// counterpart to efficiency: not "did the journey repeat one" but "was the
	// galaxy's material turned into reach or into more probes".
	double systemsPerThousandMined() const;
	// Reach, weighted by how little was squandered getting there.
	long long score() const;
	const char *grade() const;

	void printToConsole() const;
};
