// Rendersystem.cpp
#include "RenderSystem.h"
#include "Probe.h"
#include "GalaxyQuadTreeNode.h"
#include <iostream>

RenderSystem::RenderSystem(sf::RenderWindow &window) : renderWindow(window),
													   showTextLabelsStars(false),
													   showTextLabelsProbes(false),
													   showProbeTrails(false),
													   showDebugGraphics(false)
{
	// Initialize RenderSystem, if needed
	// TODO - load this from the config object somehow.
	if (!font.loadFromFile("./content/Frontier.ttf"))
	{
		std::cerr << "Error loading font file.\n";
	}
	// Configure FPS counter text //TODO - add this to debug view along with the quadtree grid.
	fpsCounter.setFont(font);
	fpsCounter.setCharacterSize(20);
	fpsCounter.setFillColor(sf::Color::White);
	fpsCounter.setPosition(10.f, 10.f); // Adjust position as needed
}

void RenderSystem::toggleTextLabelsStars()
{
	showTextLabelsStars = !showTextLabelsStars; // Toggle the flag
}

void RenderSystem::toggleTextLabelsProbes()
{
	showTextLabelsProbes = !showTextLabelsProbes; // Toggle the flag
}

void RenderSystem::toggleProbeTrails()
{
	showProbeTrails = !showProbeTrails; // Toggle the flag
}

void RenderSystem::toggleDebugGraphics()
{
	showDebugGraphics = !showDebugGraphics; // Toggle the flag
}

// Initialization - Render stars onto a texture
void RenderSystem::initializeStarsTexture(const std::vector<Star> &stars)
{
	// Create an off-screen render texture
	sf::RenderTexture renderTexture;
	renderTexture.create(renderWindow.getSize().x, renderWindow.getSize().y);
	renderTexture.clear(sf::Color::Transparent);

	// Render stars onto the texture
	for (const Star &star : stars)
	{
		// Create a base circle slightly larger and darker
		sf::CircleShape baseShape(3.0f);
		baseShape.setPosition(static_cast<float>(star.getX()), static_cast<float>(star.getY()));
		sf::Color darkerColor = star.getColour();
		darkerColor.r = std::max(0, darkerColor.r - 50);
		darkerColor.g = std::max(0, darkerColor.g - 50);
		darkerColor.b = std::max(0, darkerColor.b - 50);
		baseShape.setFillColor(darkerColor);
		renderTexture.draw(baseShape);

		// Create a core circle with the star's color
		sf::CircleShape coreShape(2.0f);
		coreShape.setPosition(static_cast<float>(star.getX()), static_cast<float>(star.getY()));
		coreShape.setFillColor(star.getColour());
		renderTexture.draw(coreShape);

		// Create a smaller circle in the center with a slightly lighter shade
		sf::CircleShape centerShape(1.0f);
		centerShape.setPosition(static_cast<float>(star.getX()), static_cast<float>(star.getY()));
		sf::Color lighterColor = star.getColour();
		lighterColor.r = std::min(255, lighterColor.r + 50);
		lighterColor.g = std::min(255, lighterColor.g + 50);
		lighterColor.b = std::min(255, lighterColor.b + 50);
		centerShape.setFillColor(lighterColor);
		renderTexture.draw(centerShape);

		if (showTextLabelsStars)
		{
			if (!star.getName().empty() && star.getName() != "\"\"") // deal with stars with blank names that get populated as ""
			{
				// Render text labels if the flag is true

				sf::Text labelText(star.getName(), font, 14);
				// std::cout << "Text Label for Star will be: " << star.getName() << std::endl; // TOO VERBOSE
				labelText.setPosition((star.getX()) + 10, (star.getY()) + 10);
				renderTexture.draw(labelText);
			}
		}
	}

	renderTexture.display();				   // dont remove this, makes texture appear upside down or go undefined position.
	starsTexture = renderTexture.getTexture(); // Save the rendered texture
}

void RenderSystem::renderStars(const std::vector<Star> &stars)
{
	sf::Sprite starsSprite(starsTexture); // Create a sprite from the pre-rendered texture
	renderWindow.draw(starsSprite);		  // Draw the sprite onto the window
}

void RenderSystem::renderProbe(const Probe &probe)
{
	if (showProbeTrails)
	{
		const std::vector<VisitedStarSystem> &probeVisitedStarSystems = probe.getVisitedStarSystems();
		sf::Color pathColor = probe.getTrailColor(); // Assuming you have a getter for the trail color in Probe

		if (probeVisitedStarSystems.size() >= 1)
		{
			for (size_t i = 1; i < probeVisitedStarSystems.size(); ++i)
			{
				const VisitedStarSystem &currentSystem = probeVisitedStarSystems[i];
				const VisitedStarSystem &prevSystem = probeVisitedStarSystems[i - 1];

				if (currentSystem.visitedByProbe && prevSystem.visitedByProbe)
				{
					// Render a line segment between the current and previous star systems
					sf::Vertex line[] = {
						sf::Vertex(prevSystem.coordinates, pathColor),
						sf::Vertex(currentSystem.coordinates, pathColor)};

					renderWindow.draw(line, 2, sf::Lines);
				}
			}
		}
	}

	// Render the probe at its current position outside the loop
	sf::CircleShape probeShape(0.5f); // Adjust the radius as needed
	probeShape.setPosition(probe.getX(), probe.getY());
	probeShape.setFillColor(sf::Color(173, 216, 230));
	renderWindow.draw(probeShape);
	if (showTextLabelsProbes)
	{
		// Render probe text label if the flag is true
		// std::string ajrtemp = probe.visitedStarSystemsToString();
		sf::Text labelText(probe.getProbeName(), font, 10);
		// sf::Text labelText(ajrtemp, font, 12);
		labelText.setPosition((probe.getX()) - 10, (probe.getY()) - 10);
		labelText.setColor(probe.getTrailColor());
		renderWindow.draw(labelText);
	}
}

void RenderSystem::renderSummaryText(const std::string &summary)
{
	summaryText.setString(summary);
	// Set the position, formatting, and other properties of the summary text
	// Example:
	summaryText.setPosition(10, 10); // Set the position of the text within the window
	renderWindow.draw(summaryText);	 // Draw the summary text within the game loop
}

void RenderSystem::calculateAndDisplayFPS()
{
	if (showDebugGraphics) // TODO - change this to have own keybind? maybe like a dev setting
	{
		// Calculate FPS
		sf::Time elapsed = fpsClock.restart();
		float fps = 1.0f / elapsed.asSeconds();

		// Update text to display FPS
		std::ostringstream ss;
		ss << "FPS: " << static_cast<int>(fps);
		fpsCounter.setString(ss.str());

		// Draw FPS counter text
		renderWindow.draw(fpsCounter);
	}
}

void RenderSystem::renderQuadtree(sf::RenderWindow &window, GalaxyQuadTreeNode *node)
{
	if (node == nullptr)
	{
		return;
	}

	if (showDebugGraphics)
	{

		sf::RectangleShape nodeRect;
		nodeRect.setSize(sf::Vector2f(node->boundary.width, node->boundary.height));
		nodeRect.setPosition(sf::Vector2f(node->boundary.left, node->boundary.top));
		nodeRect.setFillColor(sf::Color::Transparent);
		nodeRect.setOutlineThickness(0.5f);
		int outlineRed = 55; // Replace these values with your desired RGB components (0-255)
		int outlineGreen = 55;
		int outlineBlue = 55;
		int outlineAlpha = 128;
		nodeRect.setOutlineColor(sf::Color(outlineRed, outlineGreen, outlineBlue, outlineAlpha));
		window.draw(nodeRect);

		if (!node->isLeaf)
		{
			for (int i = 0; i < 4; ++i)
			{
				renderQuadtree(window, node->getChild(i));
			}
		}
	}
}