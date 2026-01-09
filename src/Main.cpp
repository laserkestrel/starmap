// Main.cpp
#include "Game.h"
#include "LoadConfig.h"
#include <iostream>
#include "DebugLog.h"

int main()
{

DEBUG_LOG("Welcome to Starmap");
DEBUG_LOG("Compiled on " << __DATE__ << " at " << __TIME__);

	// LoadConfig &myConfigInstance = LoadConfig::getInstance("./content/config.json"); // Dont supply parameter to global instance.

	LoadConfig &myConfigInstance = LoadConfig::getInstance(); // Load config usage
	DEBUG_LOG("[DEBUG] before Game ctor");
	Game myGame(myConfigInstance);
	DEBUG_LOG("[DEBUG] after Game ctor");
	myGame.run();
	return 0;
} 