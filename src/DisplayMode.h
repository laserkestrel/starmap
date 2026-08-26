// DisplayMode.h
#pragma once

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <string>

// How the window is presented.
//
//   Windowed             - a normal resizable window at the configured size.
//   BorderlessFullscreen - a borderless window filling the desktop. This is what
//                          most modern games call "fullscreen windowed": it covers
//                          the screen but alt-tabs instantly and never changes the
//                          display mode, so nothing else on the desktop is disturbed.
//   ExclusiveFullscreen  - a real fullscreen mode change. Can be marginally faster,
//                          but takes over the display and is slower to switch away from.
enum class DisplayMode
{
	Windowed,
	BorderlessFullscreen,
	ExclusiveFullscreen
};

DisplayMode displayModeFromString(const std::string &name);
const char *displayModeName(DisplayMode mode);

// The video mode and window style to create for a given display mode. The
// configured width/height are only used when windowed -- the other two take
// their size from the desktop.
sf::VideoMode videoModeFor(DisplayMode mode, unsigned int windowedWidth, unsigned int windowedHeight);
sf::Uint32 windowStyleFor(DisplayMode mode);
