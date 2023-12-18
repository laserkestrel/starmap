// Main.cpp
#include "Game.h"
#include "LoadConfig.h"
#include "iostream"

int main()
{

#if defined(_DEBUG)
	std::cout << "Hello World 1" << std::endl;
	std::cout << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl;
#endif

	LoadConfig &myConfigInstance = LoadConfig::getInstance("./content/config.json"); // Load config usage
	Game myGame(myConfigInstance);
	myGame.run();
	return 0;
}