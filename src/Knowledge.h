// Knowledge.h
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

// What a probe knows about which systems have already been visited -- by itself, or
// by any of its ancestors, and by nobody else.
//
// Deliberately NOT a global registry. A probe cannot see what an unrelated lineage
// has found, because there is no mechanism by which it could: probes separated by
// tens of parsecs have no way to talk. That is the whole point of simulating
// self-replicating probes, and it means two lineages WILL duplicate each other's
// work -- which is the interesting result, not a bug.
//
// The implementation detail that makes this affordable: ancestry is a tree, and the
// knowledge a probe inherits is frozen at the instant it forks. Its parent may learn
// more afterwards, but the child never hears about it. Immutable data can be shared
// by pointer rather than copied, so a fork costs O(1) instead of O(everything the
// ancestry ever saw) -- which is what made memory grow faster than the probe count.
class Knowledge
{
public:
	// Beyond this many links, a fork collapses the chain into one sorted block.
	//
	// Membership is tested for every candidate star in every search, so it is the
	// hot path of the whole simulation, and a long chain means walking dozens of
	// links per test. Measured at 47k probes: flattening at depth 8 took 268 ms per
	// tick, at depth 3 262 ms, at depth 1 189 ms -- and depth 1 used slightly LESS
	// memory too, because flattening deduplicates and siblings still share the one
	// flattened block. So: collapse on every fork, and membership is a single
	// binary search.
	static constexpr size_t FLATTEN_AT_DEPTH = 1;

	void learn(uint32_t starID) { learnedSince.push_back(starID); }

	bool knows(uint32_t starID) const
	{
		// Unsorted but tiny: just what this probe found since it last forked.
		for (uint32_t id : learnedSince)
		{
			if (id == starID)
				return true;
		}
		// Walked iteratively rather than recursively: a deep lineage would otherwise
		// be a stack overflow waiting to happen. Each block is sorted, so this is a
		// binary search per link rather than a scan.
		for (const Node *node = inherited.get(); node != nullptr; node = node->previous.get())
		{
			if (std::binary_search(node->ids.begin(), node->ids.end(), starID))
				return true;
		}
		return false;
	}

	// Freezes what this probe has learned into a shared immutable node and hands the
	// same node to the child. Neither side copies the ancestry; both point at it.
	void forkInto(Knowledge &child)
	{
		if (!learnedSince.empty())
		{
			std::sort(learnedSince.begin(), learnedSince.end());
			learnedSince.erase(std::unique(learnedSince.begin(), learnedSince.end()), learnedSince.end());
			inherited = std::make_shared<const Node>(inherited, std::move(learnedSince));
			learnedSince.clear();
		}
		if (chainDepth() > FLATTEN_AT_DEPTH)
		{
			inherited = flatten();
		}
		child.inherited = inherited;
		child.learnedSince.clear();
	}

	// Diagnostics: how many systems this lineage knows, and how many links the
	// lookup walks. Both are worth watching if the simulation is ever retuned.
	size_t knownCount() const
	{
		size_t total = learnedSince.size();
		for (const Node *node = inherited.get(); node != nullptr; node = node->previous.get())
		{
			total += node->ids.size();
		}
		return total;
	}
	size_t chainDepth() const
	{
		size_t depth = 0;
		for (const Node *node = inherited.get(); node != nullptr; node = node->previous.get())
		{
			++depth;
		}
		return depth;
	}

private:
	// One link: everything a single probe learned between two of its forks, kept
	// sorted so membership is a binary search. Usually a handful of ids.
	struct Node
	{
		Node(std::shared_ptr<const Node> previousNode, std::vector<uint32_t> sortedIDs)
			: previous(std::move(previousNode)), ids(std::move(sortedIDs)) {}
		std::shared_ptr<const Node> previous;
		std::vector<uint32_t> ids;
	};

	// Collapses the whole chain into a single sorted block. Costs one pass over
	// everything the lineage knows, which is why it is done rarely.
	std::shared_ptr<const Node> flatten() const
	{
		std::vector<uint32_t> all;
		all.reserve(knownCount());
		for (const Node *node = inherited.get(); node != nullptr; node = node->previous.get())
		{
			all.insert(all.end(), node->ids.begin(), node->ids.end());
		}
		std::sort(all.begin(), all.end());
		all.erase(std::unique(all.begin(), all.end()), all.end());
		return std::make_shared<const Node>(nullptr, std::move(all));
	}

	std::shared_ptr<const Node> inherited; // shared with ancestors and siblings
	std::vector<uint32_t> learnedSince;    // this probe's own additions since forking
};
