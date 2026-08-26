// SimSettings.h
#pragma once

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
};
