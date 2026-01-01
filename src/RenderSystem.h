// RenderSystem.h
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "Probe.h"
#include "Star.h" // Assuming Star class is used for rendering
#include <SFML/Graphics.hpp>
#include <sstream>
#include "GalaxyQuadTree.h"
#include "GalaxyQuadTreeNode.h"

class RenderSystem
{
public:
	RenderSystem(sf::RenderWindow &window);

	void renderStars(const std::vector<Star> &stars);
	void renderProbe(const Probe &probe); // Declaration for renderProbe
	void renderSummaryText(const std::string &summary);
	void toggleTextLabelsStars(); // Method to toggle text labels visibility
	void toggleTextLabelsProbes();
	void toggleProbeTrails();
	void toggleDebugGraphics();									 // Method to toggle probe trails visibility
	void initializeStarsTexture(const std::vector<Star> &stars); // Draw stars on a texture, then render that
	const sf::Texture &getStarsTexture() const
	{
		return starsTexture;
	}
	void calculateAndDisplayFPS();
	void renderQuadtree(sf::RenderWindow &window, GalaxyQuadTreeNode *node);

private:
	sf::RenderWindow &renderWindow;
	sf::Text summaryText; // Text object to display the summary
	sf::Font font;
	sf::Texture starsTexture;
	sf::Text fpsCounter;
	sf::Clock fpsClock;
	// void renderQuadtree(sf::RenderWindow &window, GalaxyQuadTreeNode *node);
	bool showTextLabelsStars; // Flag to control visibility of text labels
	bool showTextLabelsProbes;
	bool showProbeTrails;	// Flag to control visibility of text labels
	bool showDebugGraphics; // Flag to control visibility of FPS counter, and Quadtree boundaries
};

#endif // RENDERSYSTEM_H
