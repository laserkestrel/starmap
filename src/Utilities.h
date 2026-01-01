#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Star.h"

namespace Utilities
{
	std::string probeNamer(const std::string &str1, const std::string &str2);
	std::string getNextSequence(const std::string &sequence);

	// Add a method to populate star data
	void populateStarData(const std::vector<Star> &starVector);

	// Add a method to get star name based on ID
	std::string getStarNameFromID(uint32_t starId);

	// Define a data structure to store star ID-to-name mapping
	// std::unordered_map<uint32_t, std::string> starIdToName;

	// Other utility functions...
}

#endif // UTILITIES_H
