#include "Utilities.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>

std::unordered_map<uint32_t, std::string> starIdToName; // Add this member variable to store the star ID-to-name mapping.

void Utilities::populateStarData(const std::vector<Star> &starVector)
{
	for (const Star &star : starVector)
	{
		starIdToName[star.getID()] = star.getName();
	}
}

std::string Utilities::getStarNameFromID(uint32_t starId)
{
	auto it = starIdToName.find(starId);
	if (it != starIdToName.end())
	{
		return it->second;
	}
	return "Unknown"; // Return a default value if the ID is not found.
}

char Utilities::childLetter(int childIndex)
{
	// A..Z covers every replication limit the setup screen offers; anything beyond
	// that wraps rather than producing punctuation.
	const int i = ((childIndex % 26) + 26) % 26;
	return static_cast<char>('A' + i);
}

std::string Utilities::makeLineagePath(const std::string &parentPath, int childIndex)
{
	return parentPath + childLetter(childIndex);
}

std::string Utilities::displayStarName(const std::string &rawName, uint32_t starID)
{
	std::string name = rawName;

	// Trim whitespace; several catalogue entries carry trailing spaces.
	const size_t first = name.find_first_not_of(" \t");
	const size_t last = name.find_last_not_of(" \t");
	name = (first == std::string::npos) ? std::string() : name.substr(first, last - first + 1);

	// 84 of the nearest 2,500 stars have no name at all, and "Unknown" is what the
	// ID lookup returns when it has never heard of one. Neither makes a usable label,
	// so fall back to the catalogue number, which at least identifies the system.
	if (name.empty() || name == "Unknown")
	{
		return "#" + std::to_string(starID);
	}

	// Long proper names would swamp the label. Rare -- only 406 stars in the
	// catalogue have one -- but "Rigil Kentaurus" is 15 characters on its own.
	const size_t MaxLength = 12;
	if (name.size() > MaxLength)
	{
		name = name.substr(0, MaxLength);
	}
	return name;
}

std::string Utilities::probeLabelFor(const std::string &lineagePath, const std::string &birthStarLabel)
{
	const size_t FullPathUpTo = 6;
	const size_t TailLetters = 3;

	if (lineagePath.empty())
	{
		return "ROOT@" + birthStarLabel;
	}
	if (lineagePath.size() <= FullPathUpTo)
	{
		return lineagePath + "@" + birthStarLabel;
	}
	return std::to_string(lineagePath.size()) + ":" +
		   lineagePath.substr(lineagePath.size() - TailLetters) + "@" + birthStarLabel;
}

std::string Utilities::makeProbeName(const std::string &parentPath, int childIndex,
									 const std::string &birthStarName, uint32_t birthStarID)
{
	return makeLineagePath(parentPath, childIndex) + "@" + displayStarName(birthStarName, birthStarID);
}
