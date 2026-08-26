// DisplayMode.cpp
#include "DisplayMode.h"
#include <iostream>

DisplayMode displayModeFromString(const std::string &name)
{
	if (name == "windowed") return DisplayMode::Windowed;
	if (name == "borderless" || name == "borderlessFullscreen") return DisplayMode::BorderlessFullscreen;
	if (name == "fullscreen" || name == "exclusiveFullscreen") return DisplayMode::ExclusiveFullscreen;
	std::cerr << "Config: unknown displayMode '" << name << "', falling back to borderlessFullscreen." << std::endl;
	return DisplayMode::BorderlessFullscreen;
}

const char *displayModeName(DisplayMode mode)
{
	switch (mode)
	{
	case DisplayMode::Windowed: return "windowed";
	case DisplayMode::BorderlessFullscreen: return "borderlessFullscreen";
	case DisplayMode::ExclusiveFullscreen: return "exclusiveFullscreen";
	default: return "borderlessFullscreen";
	}
}

sf::VideoMode videoModeFor(DisplayMode mode, unsigned int windowedWidth, unsigned int windowedHeight)
{
	if (mode == DisplayMode::Windowed)
	{
		return sf::VideoMode(windowedWidth, windowedHeight);
	}

	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

	// Exclusive fullscreen needs a mode the driver actually advertises. The desktop
	// mode almost always qualifies, but fall back to the best reported one if not
	// rather than letting SFML fail to create the window.
	if (mode == DisplayMode::ExclusiveFullscreen && !desktop.isValid())
	{
		const auto &modes = sf::VideoMode::getFullscreenModes();
		if (!modes.empty())
		{
			desktop = modes.front();
		}
	}
	return desktop;
}

sf::Uint32 windowStyleFor(DisplayMode mode)
{
	switch (mode)
	{
	case DisplayMode::Windowed: return sf::Style::Default;
	case DisplayMode::BorderlessFullscreen: return sf::Style::None;
	case DisplayMode::ExclusiveFullscreen: return sf::Style::Fullscreen;
	default: return sf::Style::None;
	}
}
