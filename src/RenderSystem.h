// RenderSystem.h
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "Probe.h"
#include "Projection.h"
#include "Star.h"
#include "GalaxyQuadTree.h"
#include "GalaxyQuadTreeNode.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class RenderSystem
{
public:
	RenderSystem(sf::RenderWindow &window, const Projection &projection);

	// Redraws the whole starfield into an off-screen texture: the reference grid
	// on the z = 0 plane, one stalk per star showing its height above or below
	// that plane, then the stars themselves painted far-to-near.
	void initializeStarsTexture(const std::vector<Star> &stars);

	void renderProbes(const std::vector<Probe> &probes);
	void renderSummaryText(const std::string &summary);
	void renderParameterList(const std::vector<std::pair<std::string, std::string>> &params, int focusedIndex, bool showCaret);
	void renderQuadtree(sf::RenderWindow &window, const GalaxyQuadTreeNode *node);
	void calculateAndDisplayFPS();

	void toggleTextLabelsStars();
	void toggleTextLabelsProbes();
	void toggleProbeTrails();
	void toggleDebugGraphics();
	void toggleStarStalks();

	bool getShowTextLabelsStars() const { return showTextLabelsStars; }
	const sf::Texture &getStarsTexture() const { return starsTexture; }

private:
	sf::RenderWindow &renderWindow;
	const Projection &projection;
	sf::Text summaryText;
	sf::Font font;
	sf::Texture starsTexture;
	sf::Text fpsCounter;
	sf::Clock fpsClock;

	bool showTextLabelsStars = false;
	bool showTextLabelsProbes = false;
	bool showProbeTrails = false;
	bool showDebugGraphics = false;
	bool showStarStalks = true; // the stalks are the whole point of the tilted view
};

#endif // RENDERSYSTEM_H
