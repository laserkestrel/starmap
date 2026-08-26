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

long long RunMetrics::score() const
{
	// Systems discovered, scaled by the share of journeys that were not wasted.
	// Reaching a lot by flailing scores no better than reaching less, cleanly.
	return static_cast<long long>(std::llround(static_cast<double>(uniqueSystems) * efficiency()));
}

const char *RunMetrics::grade() const
{
	const double e = efficiency();
	const double c = coveragePercent();
	if (e >= 0.85 && c >= 20.0) return "A";
	if (e >= 0.70 && c >= 10.0) return "B";
	if (e >= 0.55 && c >= 4.0)  return "C";
	if (e >= 0.40)              return "D";
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
			  << "           nothing within range       " << stoppedWithNothingInRange << "\n";

	std::cout << "RESULT\n"
			  << "  score                    " << score() << "\n"
			  << "  grade                    " << grade() << "\n"
			  << "-----------------------------------------------------------\n"
			  << "(wall clock " << wallClockSeconds << "s -- for interest only, no metric above uses it)\n"
			  << "===========================================================" << std::endl;
}
