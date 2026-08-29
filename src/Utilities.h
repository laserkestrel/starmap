#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "Star.h"

namespace Utilities
{
	// --- probe naming -----------------------------------------------------------
	// A probe is named for its path through the family tree and the system it was
	// built at: "ABCAB@Gl 65A". One letter per generation says which child it was, so
	//
	//   - the name is unique by construction,
	//   - its LENGTH is the generation, readable at a glance,
	//   - a shared prefix means shared ancestry: ABCAB and ABCAA are siblings,
	//     ABCAB and BAA parted company at the root.
	//
	// This replaces "<birth star, 3 chars>-<parent's birth star, 3 chars>-<code>",
	// which had three separate problems. The code was not an identifier at all but the
	// generation depth, arrived at by incrementing the parent's, so every probe at a
	// given depth carried the same one and names were never unique. Three characters
	// collided constantly -- "Gl 65A" and "Gl 244B" both truncate to "GL ". And 84 of
	// the nearest 2,500 stars have no name in the catalogue, which left that group
	// empty and produced names beginning with a bare dash.

	// The letter for a probe that is its parent's `childIndex`-th copy.
	char childLetter(int childIndex);

	// Full name for a new probe. `parentPath` is the parent's path ("" for the root's
	// children), `birthStarName` is where it is being built, `birthStarID` is the
	// fallback for the many catalogue entries with no name at all.
	std::string makeProbeName(const std::string &parentPath, int childIndex,
							  const std::string &birthStarName, uint32_t birthStarID);

	// Just the path part, so a child can extend it without re-parsing the name.
	std::string makeLineagePath(const std::string &parentPath, int childIndex);

	// A star's name trimmed to something that fits on a label, falling back to its
	// catalogue ID when the entry is unnamed.
	std::string displayStarName(const std::string &rawName, uint32_t starID);

	// The on-screen form of a name. The full path is the identity and stays intact in
	// the console and the debrief, but it gets long: the family tree turns out to be
	// long and thin rather than bushy -- a resource economy that only just affords one
	// copy per probe produces chains twenty generations deep, where a tree with every
	// probe making three copies would be about seven. So paths past a handful of
	// letters are shown as generation plus the last few branches, which is what tells
	// you where a probe sits without needing the whole ancestry on the map.
	std::string probeLabelFor(const std::string &lineagePath, const std::string &birthStarLabel);

	// Add a method to populate star data
	void populateStarData(const std::vector<Star> &starVector);

	// Add a method to get star name based on ID
	std::string getStarNameFromID(uint32_t starId);

	// Other utility functions...
}

#endif // UTILITIES_H
