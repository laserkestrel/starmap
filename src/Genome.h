// Genome.h
#pragma once

#include <algorithm>
#include <cstddef>
#include <random>

// The behavioural traits a probe carries and passes to its children.
//
// These used to live in SimSettings as fleet-wide law: every probe searched the same
// distance, kept the same share of fuel for its children, replicated the same number
// of times. Making them per-probe and heritable turns the setup screen from "how
// probes behave" into "what the FIRST probe believes", and lets the rest be settled
// by whether a strategy produces surviving descendants.
//
// There is deliberately no fitness function anywhere. Selection is already in the
// simulation and has been since replication started costing resources: a probe that
// over-reaches strands and stops reproducing, one that hoards gets outbred, one that
// mines a poor system to the last drop wastes ticks a rival spends travelling. The
// work is not to implement selection, it is to stop preventing it.
struct Genome
{
	float searchRadiusParsecs = 8.0f;  // reach, against the risk of stranding
	float childFuelShare = 0.35f;      // parental investment: few well-fuelled, or many on fumes
	float replicationLimit = 3.0f;     // copies before shutting down (used as an integer)
	float harvestPatience = 60.0f;     // ticks to spend on one system before moving on

	int replicationLimitInt() const { return static_cast<int>(std::lround(replicationLimit)); }
	int harvestPatienceInt() const { return std::max(1, static_cast<int>(std::lround(harvestPatience))); }
};

// Trait metadata, kept in one place so the debrief, the colour ramp and the mutation
// clamps cannot disagree about what a sensible value is.
enum class Trait
{
	SearchRadius = 0,
	ChildFuelShare,
	ReplicationLimit,
	HarvestPatience,
	TraitCount
};

struct TraitInfo
{
	const char *name;
	const char *shortName;
	float minValue;
	float maxValue;
	int decimals;
};

inline const TraitInfo &traitInfo(Trait t)
{
	// Ranges are the same ones the setup sliders offer, so an evolved value is always
	// something you could have dialled in by hand.
	static const TraitInfo INFO[] = {
		{"Search radius",   "radius",   1.0f,  30.0f, 2},
		{"Child fuel share","fuel",     0.05f,  0.90f, 3},
		{"Replication limit","repl",    0.0f,  10.0f, 2},
		{"Harvest patience","patience", 5.0f, 200.0f, 1},
	};
	const size_t i = static_cast<size_t>(t);
	return INFO[i < static_cast<size_t>(Trait::TraitCount) ? i : 0];
}

inline float traitValue(const Genome &g, Trait t)
{
	switch (t)
	{
	case Trait::SearchRadius:    return g.searchRadiusParsecs;
	case Trait::ChildFuelShare:  return g.childFuelShare;
	case Trait::ReplicationLimit:return g.replicationLimit;
	case Trait::HarvestPatience: return g.harvestPatience;
	default:                     return 0.0f;
	}
}

inline void setTraitValue(Genome &g, Trait t, float v)
{
	switch (t)
	{
	case Trait::SearchRadius:     g.searchRadiusParsecs = v; break;
	case Trait::ChildFuelShare:   g.childFuelShare = v; break;
	case Trait::ReplicationLimit: g.replicationLimit = v; break;
	case Trait::HarvestPatience:  g.harvestPatience = v; break;
	default: break;
	}
}

// Where a value sits in its own range, 0..1. Used to colour a trail by trait, so the
// same ramp works for a radius in parsecs and a share between nought and one.
inline float traitFraction(const Genome &g, Trait t)
{
	const TraitInfo &info = traitInfo(t);
	if (info.maxValue <= info.minValue)
		return 0.0f;
	const float f = (traitValue(g, t) - info.minValue) / (info.maxValue - info.minValue);
	return std::max(0.0f, std::min(1.0f, f));
}

// A child's genome: the parent's, nudged. `strength` is the largest proportional
// change a single trait can take in one generation, so 0.08 means up to plus or minus
// eight percent. Multiplicative rather than additive so the same figure is sensible
// for a radius measured in parsecs and a share measured in tenths.
//
// Starting from ONE founder means every bit of variation in the run has to come from
// here -- there is no standing diversity to select from -- so this matters more than
// it would in a population seeded with variety.
inline Genome mutated(const Genome &parent, float strength, std::mt19937 &rng)
{
	if (strength <= 0.0f)
		return parent;

	std::uniform_real_distribution<float> jitter(-strength, strength);
	Genome child = parent;
	for (int i = 0; i < static_cast<int>(Trait::TraitCount); ++i)
	{
		const Trait t = static_cast<Trait>(i);
		const TraitInfo &info = traitInfo(t);
		float v = traitValue(parent, t) * (1.0f + jitter(rng));

		// A trait sitting at zero can never move under a multiplicative nudge, so give
		// it a floor to climb off. Without this a replication limit that reached 0
		// would be an inescapable dead end for that whole lineage.
		if (v < info.minValue)
			v = info.minValue;
		if (v > info.maxValue)
			v = info.maxValue;
		setTraitValue(child, t, v);
	}
	return child;
}
