// HighScores.h
#pragma once

#include "RunMetrics.h"
#include <cstddef>
#include <string>
#include <vector>

// One completed expedition, kept in ./content/highscores.json so results survive
// between launches for the life of the installation.
//
// The parameters are stored alongside the result deliberately: a leaderboard of
// bare numbers tells you which run won, but not what it did differently. With the
// settings recorded, the table becomes the comparison tool the project is for.
struct HighScoreEntry
{
	long long score = 0;
	std::string grade = "E";
	long long systems = 0;
	double coveragePercent = 0.0;
	double efficiency = 0.0;
	long long ticks = 0;
	long long probesBuilt = 0;
	double frontierParsecs = 0.0;
	std::string endReason;
	std::string when;

	// what it was run with
	float searchRadiusParsecs = 0.0f;
	int replicationLimit = 0;
	float probeSpeed = 0.0f;
	int starsLoaded = 0;
	bool replicateOnFirstArrival = false;
};

class HighScores
{
public:
	static const size_t MaxEntries = 12;

	// Missing or unreadable file is not an error -- you simply have no scores yet.
	void load(const std::string &path = "./content/highscores.json");
	void save() const;

	// Inserts, sorts by score and trims. Returns the 1-based rank it landed at, or
	// 0 if it did not make the table.
	int add(const HighScoreEntry &entry);

	const std::vector<HighScoreEntry> &entries() const { return table; }
	long long bestScore() const { return table.empty() ? 0 : table.front().score; }

	// Builds an entry from a finished run plus the settings that produced it.
	static HighScoreEntry fromRun(const RunMetrics &metrics, float searchRadius, int replicationLimit,
								  float probeSpeed, int starsLoaded, bool replicateOnFirstArrival);

private:
	std::vector<HighScoreEntry> table;
	std::string filePath = "./content/highscores.json";
};
