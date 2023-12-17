//RenderSystem.h
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "Probe.h"
#include "Star.h" // Assuming Star class is used for rendering
#include <SFML/Graphics.hpp>

class RenderSystem
{
public:
	RenderSystem(sf::RenderWindow& window);

	void renderStars(const std::vector<Star>& stars);
	void renderProbe(const Probe& probe); // Declaration for renderProbe
	//void renderProbePath(const Probe& probe);
	void renderSummaryText(const std::string& summary);
	void toggleTextLabels(); // Method to toggle text labels visibility
	void toggleProbeTrails(); // Method to toggle probe trails visibility

private:
	sf::RenderWindow& renderWindow;
	sf::Text summaryText; // Text object to display the summary
	sf::Font font;
	bool showTextLabels; // Flag to control visibility of text labels
	bool showProbeTrails; // Flag to control visibility of text labels
};

#endif // RENDERSYSTEM_H
