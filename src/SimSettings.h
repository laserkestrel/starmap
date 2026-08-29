// SimSettings.h
#pragma once

#include "Resources.h"
#include "Genome.h"

// Runtime simulation settings.
//
// These start from config.json but can be edited on the startup screen, so they
// live in one mutable struct that Game owns and probes hold a pointer to. That
// replaces the old arrangement where Game kept an edited copy while Probe read
// the original straight out of the LoadConfig singleton -- which is why editing
// the search radius on the startup screen used to have no effect on the probes.
struct SimSettings
{
	float probeSearchRadiusParsecs = 8.0f;
	float probeSpeedParsecsPerTick = 0.25f;
	int probeIndividualReplicationLimit = 3;
	// Whether a probe may copy itself at the very first system it reaches, or has to
	// establish itself there first and wait for the next one. A real lever on the
	// growth curve: switching it on roughly doubles the exponent.
	bool replicateOnFirstArrival = false;

	// --- the resource economy ---------------------------------------------------
	// Off restores the old behaviour: replication is free, and the population cap is
	// the only thing that ends a run. Kept switchable so the two can be compared.
	bool resourcesEnabled = true;
	Resources replicationCost{55.0f, 35.0f, 8.0f};
	float harvestPerTick = 6.0f;
	int maxHarvestTicks = 60;
	float fuelPerParsec = 1.5f;
	float fuelSafetyMargin = 1.25f;
	float childFuelShare = 0.35f;

	// --- evolution ---------------------------------------------------------------
	// Off, every probe uses the founder's genome unchanged and the simulation behaves
	// as it did before. On, a child's traits are its parent's nudged by up to
	// mutationStrength either way, and nothing else selects: whichever values produce
	// surviving descendants become common.
	bool evolutionEnabled = true;
	// The control condition. Traits still mutate and are still recorded, but every
	// probe BEHAVES as the founder did, so the genome cannot affect who survives.
	// Anything the trait report shows in this mode is drift, measured in the real
	// branching structure of a real run rather than approximated outside it. It is
	// the only way to know whether a move under selection means anything.
	bool neutralControl = false;
	float mutationStrength = 0.08f;
	// What the first probe believes. The setup sliders write here, so they set the
	// starting point rather than fleet-wide law.
	Genome founderGenome;

	// How much of the colour wheel the whole family tree spans. 1.0 uses all of it,
	// which separates the top-level branches as far as possible; lower values tint
	// the entire fleet towards one part of the spectrum.
	float lineageHueSpread = 1.0f;

	// Advanced by Game once per tick. Probes stamp their arrivals with it so the
	// renderer can tell a leg flown moments ago from one flown a thousand ticks back.
	long long currentTick = 0;
};
