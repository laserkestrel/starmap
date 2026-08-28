// HighScores.cpp
#include "HighScores.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	std::string nowStamp()
	{
		const std::time_t t = std::time(nullptr);
		std::tm tm{};
#if defined(_WIN32)
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif
		char buffer[32] = {0};
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &tm);
		return std::string(buffer);
	}
} // namespace

void HighScores::load(const std::string &path)
{
	filePath = path;
	table.clear();

	std::ifstream file(filePath);
	if (!file.is_open())
	{
		// Entirely normal on a first run. Say so plainly rather than as an error.
		std::cout << "No high score file yet at " << filePath << " -- one will be written after this run." << std::endl;
		return;
	}

	try
	{
		json root;
		file >> root;
		if (!root.is_array())
		{
			std::cerr << "High score file is not a list; ignoring it." << std::endl;
			return;
		}
		for (const auto &e : root)
		{
			HighScoreEntry entry;
			entry.score = e.value("score", 0LL);
			entry.grade = e.value("grade", std::string("E"));
			entry.systems = e.value("systems", 0LL);
			entry.coveragePercent = e.value("coveragePercent", 0.0);
			entry.efficiency = e.value("efficiency", 0.0);
			entry.ticks = e.value("ticks", 0LL);
			entry.probesBuilt = e.value("probesBuilt", 0LL);
			entry.frontierParsecs = e.value("frontierParsecs", 0.0);
			entry.endReason = e.value("endReason", std::string());
			entry.when = e.value("when", std::string());
			entry.searchRadiusParsecs = e.value("searchRadiusParsecs", 0.0f);
			entry.replicationLimit = e.value("replicationLimit", 0);
			entry.probeSpeed = e.value("probeSpeed", 0.0f);
			entry.starsLoaded = e.value("starsLoaded", 0);
			entry.replicateOnFirstArrival = e.value("replicateOnFirstArrival", false);
			table.push_back(entry);
		}
		std::sort(table.begin(), table.end(),
				  [](const HighScoreEntry &a, const HighScoreEntry &b) { return a.score > b.score; });
		if (table.size() > MaxEntries)
			table.resize(MaxEntries);
		std::cout << "Loaded " << table.size() << " high scores from " << filePath << "." << std::endl;
	}
	catch (const json::exception &e)
	{
		std::cerr << "Could not parse " << filePath << " (" << e.what() << ") -- starting a fresh table." << std::endl;
		table.clear();
	}
}

void HighScores::save() const
{
	json root = json::array();
	for (const auto &entry : table)
	{
		root.push_back({{"score", entry.score},
						{"grade", entry.grade},
						{"systems", entry.systems},
						{"coveragePercent", entry.coveragePercent},
						{"efficiency", entry.efficiency},
						{"ticks", entry.ticks},
						{"probesBuilt", entry.probesBuilt},
						{"frontierParsecs", entry.frontierParsecs},
						{"endReason", entry.endReason},
						{"when", entry.when},
						{"searchRadiusParsecs", entry.searchRadiusParsecs},
						{"replicationLimit", entry.replicationLimit},
						{"probeSpeed", entry.probeSpeed},
						{"starsLoaded", entry.starsLoaded},
						{"replicateOnFirstArrival", entry.replicateOnFirstArrival}});
	}

	std::ofstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Could not write high scores to " << filePath << "." << std::endl;
		return;
	}
	file << root.dump(2) << std::endl;
}

int HighScores::add(const HighScoreEntry &entry)
{
	table.push_back(entry);
	std::stable_sort(table.begin(), table.end(),
					 [](const HighScoreEntry &a, const HighScoreEntry &b) { return a.score > b.score; });
	if (table.size() > MaxEntries)
		table.resize(MaxEntries);

	for (size_t i = 0; i < table.size(); ++i)
	{
		// Identified by its timestamp and score together -- two runs can tie.
		if (table[i].when == entry.when && table[i].score == entry.score &&
			table[i].ticks == entry.ticks)
		{
			return static_cast<int>(i) + 1;
		}
	}
	return 0;
}

HighScoreEntry HighScores::fromRun(const RunMetrics &metrics, float searchRadius, int replicationLimit,
								   float probeSpeed, int starsLoaded, bool replicateOnFirstArrival)
{
	HighScoreEntry entry;
	entry.score = metrics.score();
	entry.grade = metrics.grade();
	entry.systems = metrics.uniqueSystems;
	entry.coveragePercent = metrics.coveragePercent();
	entry.efficiency = metrics.efficiency();
	entry.ticks = metrics.ticks;
	entry.probesBuilt = metrics.probesBuilt;
	entry.frontierParsecs = metrics.frontierParsecs;
	entry.endReason = runEndReasonText(metrics.endReason);
	entry.when = nowStamp();
	entry.searchRadiusParsecs = searchRadius;
	entry.replicationLimit = replicationLimit;
	entry.probeSpeed = probeSpeed;
	entry.starsLoaded = starsLoaded;
	entry.replicateOnFirstArrival = replicateOnFirstArrival;
	return entry;
}
