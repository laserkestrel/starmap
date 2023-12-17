#include "Utilities.h"
#include <iostream>
#include <string>
#include <algorithm>

std::string Utilities::probeNamer(const std::string& str1, const std::string& str2)
{
	/*
	Each group (GRP) is 3 characters (Alphabetical A-Z)
	Naming standard GRP1-GRP2-GRP3
	GRP1 is parent probe name (tokens 1,2,3)
	GRP2 is parent probe replication location (or this/self birthplace) (token 1,2,3)
	GRP3 is a 3 character alphanumeric generation ID, consists A-Z, supports upto 17576 generations which can be derrived from parent probe name, if it expires, gets stuck at ZZZ)
	Example: Probe from Sol to Proxima Centauri is SOL-SOL-AAA > replicates to PRO-SOL-AAB

	*/
	std::string abvParentBirplace = str1.substr(0, 3); //String 1 passed in from game is probe.getProbeName()
	std::string abvChildBirplace = str2.substr(0, 3);  // String 2 passed in from game is probe.getTargetStar()
	std::string abvGenerationID = str1.substr(str1.length() - 3);
	std::string currentSequence = abvGenerationID;
	std::string nextSequence = getNextSequence(currentSequence);
	std::string newName = abvChildBirplace + "-" + abvParentBirplace + "-" + nextSequence;
	std::transform(newName.begin(), newName.end(), newName.begin(), ::toupper); // Convert to uppercase
	return newName;
}

std::string Utilities::getNextSequence(const std::string& sequence)
{
	std::string result = sequence;

	// Increment the sequence
	if (sequence[2] < 'Z')
	{
		result[2]++;
	}
	else if (sequence[1] < 'Z')
	{
		result[1]++;
		result[2] = 'A';
	}
	else if (sequence[0] < 'Z')
	{
		result[0]++;
		result[1] = 'A';
		result[2] = 'A';
	}
	// Handle any other cases based on your requirements

	return result;
}