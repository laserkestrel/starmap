// RunMetrics.cpp
#include "RunMetrics.h"
#include <cmath>
#include <iomanip>
#include <iostream>

const char *runEndReasonText(RunEndReason reason)
{
	switch (reason)
	{
	case RunEndReason::AllProbesStopped: return "every probe stopped";
	case RunEndReason::CoverageReached:  return "coverage target reached";
	case RunEndReason::FrontierStalled:  return "frontier stopped advancing";
	case RunEndReason::IterationLimit:   return "tick limit reached";
	case RunEndReason::PopulationCap:    return "fleet hit the population cap";
	case RunEndReason::Abandoned:        return "abandoned";
	default:                             return "still running";
	}
}

double RunMetrics::coveragePercent() const
{
	if (catalogueSize == 0)
		return 0.0;
	return 100.0 * static_cast<double>(uniqueSystems) / static_cast<double>(catalogueSize);
}

double RunMetrics::efficiency() const
{
	if (arrivals <= 0)
		return 0.0;
	return static_cast<double>(uniqueSystems) / static_cast<double>(arrivals);
}

long long RunMetrics::wastedJourneys() const
{
	return arrivals > uniqueSystems ? arrivals - uniqueSystems : 0;
}

double RunMetrics::parsecsPerDiscovery() const
{
	if (uniqueSystems <= 0)
		return 0.0;
	return distanceFlownParsecs / static_cast<double>(uniqueSystems);
}

double RunMetrics::probesPerDiscovery() const
{
	if (uniqueSystems <= 0)
		return 0.0;
	return static_cast<double>(probesBuilt) / static_cast<double>(uniqueSystems);
}

double RunMetrics::expansionRatePerThousandTicks() const
{
	if (ticks <= 0)
		return 0.0;
	return frontierParsecs / static_cast<double>(ticks) * 1000.0;
}

int RunMetrics::lastPopulousGeneration(size_t minimumPopulation) const
{
	for (int g = static_cast<int>(generations.size()) - 1; g >= 0; --g)
	{
		if (generations[static_cast<size_t>(g)].population >= minimumPopulation)
			return g;
	}
	return generations.empty() ? 0 : static_cast<int>(generations.size()) - 1;
}

double RunMetrics::systemsPerThousandMined() const
{
	const double mined = static_cast<double>(totalMined.total());
	if (mined <= 0.0)
		return 0.0;
	return 1000.0 * static_cast<double>(uniqueSystems) / mined;
}

long long RunMetrics::score() const
{
	// Half the credit for reaching a system at all, half for not having wasted a
	// journey doing it.
	//
	// This used to be reach x efficiency outright, which sounds fair and is not: it
	// made efficiency the whole game. Measured over a 2,000 star catalogue, a fleet
	// held to one copy each reached 38% of it with a perfect 1.00 -- every probe
	// dies young, so lineages never overlap -- while a fleet allowed two copies
	// reached 92% at 0.10. The old formula scored those 767 against 187, which says
	// the sprawling run was four times worse despite reaching two and a half times
	// as much of the galaxy. Nobody would choose to sprawl, so there was no decision
	// left to make. Floored at a half, both are playable and the better choice
	// genuinely depends on the galaxy -- careful wins in a small one it can finish,
	// aggressive wins in a large one it cannot.
	const double weighted = static_cast<double>(uniqueSystems) * (0.5 + 0.5 * efficiency());
	return static_cast<long long>(std::llround(weighted));
}

const char *RunMetrics::grade() const
{
	// Graded against the perfect run for THIS catalogue -- every system reached, no
	// journey wasted -- so a grade means the same thing whether the galaxy holds 400
	// stars or 50,000. Absolute thresholds could not do that: any fixed coverage
	// figure is trivial in a small catalogue and unreachable in a large one.
	if (catalogueSize == 0)
		return "E";

	const double achieved = static_cast<double>(score()) / static_cast<double>(catalogueSize);
	if (achieved >= 0.80) return "A";
	if (achieved >= 0.60) return "B";
	if (achieved >= 0.40) return "C";
	if (achieved >= 0.20) return "D";
	return "E";
}

void RunMetrics::printToConsole() const
{
	// Kept alongside the on-screen debrief so headless runs, CI and piping to a
	// file still produce the full picture.
	std::cout << "\n=================== EXPEDITION COMPLETE ===================\n"
			  << "Ended because: " << runEndReasonText(endReason) << "  after " << ticks << " ticks\n"
			  << "-----------------------------------------------------------\n";
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "REACH\n"
			  << "  systems reached          " << uniqueSystems << " of " << catalogueSize
			  << "  (" << coveragePercent() << "%)\n"
			  << "  frontier                 " << frontierParsecs << " pc from Sol\n"
			  << "  expansion rate           " << expansionRatePerThousandTicks() << " pc per 1000 ticks\n";
	if (ticksTo25 >= 0) std::cout << "  ticks to 25% coverage    " << ticksTo25 << "\n";
	if (ticksTo50 >= 0) std::cout << "  ticks to 50% coverage    " << ticksTo50 << "\n";
	if (ticksTo75 >= 0) std::cout << "  ticks to 75% coverage    " << ticksTo75 << "\n";

	std::cout << "EFFICIENCY\n"
			  << "  efficiency               " << efficiency()
			  << "   (1.00 = no journey wasted)\n"
			  << "  wasted journeys          " << wastedJourneys() << " of " << arrivals << " arrivals\n"
			  << "  cost per system          " << parsecsPerDiscovery() << " pc flown, "
			  << probesPerDiscovery() << " probes built\n";

	std::cout << "FLEET\n"
			  << "  probes built             " << probesBuilt << "\n"
			  << "  peak population          " << peakPopulation << "\n"
			  << "  still alive at the end   " << probesAlive << "\n"
			  << "  stopped: replication limit reached  " << stoppedAtReplicationLimit << "\n"
			  << "           nothing within range       " << stoppedWithNothingInRange << "\n"
			  << "           stranded, out of fuel      " << stoppedStranded << "\n";

	if (resourcesEnabled)
	{
		std::cout << "ECONOMY\n"
				  << "  mined  metals/volatiles/fissiles  " << totalMined.metals << " / "
				  << totalMined.volatiles << " / " << totalMined.fissiles << "\n"
				  << "  spent building probes             " << resourcesSpentOnProbes.total() << "\n"
				  << "  systems stripped bare             " << systemsExhausted << "\n"
				  << "  probe-ticks spent mining          " << harvestTicks << "\n"
				  << "  systems per 1000 mined            " << systemsPerThousandMined() << "\n";
	}

	if (evolutionEnabled && !generations.empty())
	{
		const int last = lastPopulousGeneration();
		const Genome &start = founderGenome;
		const Genome &end = generations[static_cast<size_t>(last)].mean;
		std::cout << "EVOLUTION  (mutation " << (mutationStrength * 100.0f) << "% per generation)\n";
		for (int i = 0; i < static_cast<int>(Trait::TraitCount); ++i)
		{
			const Trait t = static_cast<Trait>(i);
			const TraitInfo &info = traitInfo(t);
			const float a = traitValue(start, t);
			const float b = traitValue(end, t);
			const float change = (a != 0.0f) ? 100.0f * (b - a) / a : 0.0f;
			std::cout << "  " << info.name;
			for (size_t pad = std::string(info.name).size(); pad < 22; ++pad) std::cout << ' ';
			std::cout << "founder " << a << "  ->  gen " << last << " " << b
					  << "   (" << (change >= 0 ? "+" : "") << change << "%)\n";
		}
		std::cout << "  generations reached      " << (generations.size() - 1) << "\n";
	}

	std::cout << "RESULT\n"
			  << "  score                    " << score() << "\n"
			  << "  grade                    " << grade() << "\n"
			  << "-----------------------------------------------------------\n"
			  << "(wall clock " << wallClockSeconds << "s -- for interest only, no metric above uses it)\n"
			  << "===========================================================" << std::endl;
}
