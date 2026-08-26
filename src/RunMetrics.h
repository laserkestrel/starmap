// RunMetrics.h
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

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
	double wallClockSeconds = 0.0; // reported for interest only, never used in a metric
	RunEndReason endReason = RunEndReason::StillRunning;

	// Ticks taken to reach each quarter of the catalogue; -1 if never reached.
	long long ticksTo25 = -1, ticksTo50 = -1, ticksTo75 = -1;

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
	// Reach, weighted by how little was squandered getting there.
	long long score() const;
	const char *grade() const;

	void printToConsole() const;
};
