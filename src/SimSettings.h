// SimSettings.h
#pragma once

#include "Resources.h"

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

	// Advanced by Game once per tick. Probes stamp their arrivals with it so the
	// renderer can tell a leg flown moments ago from one flown a thousand ticks back.
	long long currentTick = 0;
};
