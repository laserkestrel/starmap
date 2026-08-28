// RenderSystem.h
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "GalaxyQuadTree.h"
#include "GalaxyQuadTreeNode.h"
#include "Probe.h"
#include "Projection.h"
#include "HighScores.h"
#include "RunMetrics.h"
#include "Star.h"
#include "StarSprite.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

class RenderSystem
{
public:
	RenderSystem(sf::RenderWindow &window, const Projection &projection, StarSpriteStyle spriteStyle);

	// Draws the whole starfield for the current camera: the reference grid on the
	// z = 0 plane, a stalk per star showing its height above or below that plane,
	// then the stars themselves as additively blended sprites, painted far to near.
	//
	// This runs every frame rather than baking into a texture, because a baked
	// texture cannot pan or zoom. The quadtree supplies only the stars that could
	// be on screen, so the cost tracks what is visible, not the catalogue size.
	void renderStarfield(const std::vector<Star> &stars, const GalaxyQuadTree &quadTree);

	void renderProbes(const std::vector<Probe> &probes);
	void renderSummaryText(const std::string &summary);
	void renderQuadtree(sf::RenderWindow &window, const GalaxyQuadTreeNode *node);
	void renderHud(const std::string &text);
	// Where the debrief's buttons ended up, so Game can hit-test them.
	struct DebriefButtons
	{
		sf::FloatRect again;
		sf::FloatRect close;
		sf::FloatRect quit;
	};

	// End-of-run debrief, drawn over the live map. `newRank` is the 1-based place
	// this run took on the table, or 0 if it did not place.
	void renderDebrief(const RunMetrics &metrics, const HighScores &scores, int newRank);
	const DebriefButtons &getDebriefButtons() const { return debriefButtons; }
	void calculateAndDisplayFPS();

	void toggleTextLabelsStars();
	void toggleTextLabelsProbes();
	void toggleProbeTrails();
	void toggleDebugGraphics();
	void toggleStarStalks();

	// Recreating the window can invalidate the GL resources these hold, so the
	// font and sprite are rebuilt after any display-mode change.
	void reloadGraphicsResources();

	// How many labels to draw. The brightest visible stars win, so names thin out
	// gracefully as you zoom out rather than disappearing all at once.
	void setLabelMaxVisible(size_t limit) { labelMaxVisible = limit; }
	size_t getLabelMaxVisible() const { return labelMaxVisible; }

	void setSpriteStyle(StarSpriteStyle style);
	StarSpriteStyle getSpriteStyle() const { return spriteStyle; }
	StarSpriteStyle cycleSpriteStyle();

	size_t getLastVisibleStarCount() const { return lastVisibleStarCount; }
	// The setup screen draws its own controls and needs the loaded font.
	const sf::Font &getFont() const { return font; }

private:
	void appendSpriteQuad(sf::VertexArray &target, const sf::Vector2f &centre, float sizePixels, const sf::Color &colour) const;

	sf::RenderWindow &renderWindow;
	const Projection &projection;

	sf::Font font;
	sf::Text summaryText;
	sf::Text fpsCounter;
	sf::Text hudText;
	sf::Clock fpsClock;

	StarSpriteStyle spriteStyle = StarSpriteStyle::CoreHalo;
	sf::Texture starSprite;

	// Reused between frames so a moving camera does not reallocate every tick.
	std::vector<size_t> visibleScratch;
	std::vector<size_t> visible;
	std::vector<size_t> labelCandidates;
	// Screen cells already carrying a probe name this frame. Kept as a member for
	// the same reason as the vectors above: cleared and refilled, never reallocated.
	std::unordered_set<int> occupiedLabelCells;
	sf::VertexArray gridLines{sf::Lines};
	sf::VertexArray stalkLines{sf::Lines};
	sf::VertexArray starQuads{sf::Quads};
	sf::VertexArray probeQuads{sf::Quads};
	sf::VertexArray trailLines{sf::Lines};

	size_t lastVisibleStarCount = 0;
	size_t labelMaxVisible = 120;
	DebriefButtons debriefButtons;

	bool showTextLabelsStars = false;
	bool showTextLabelsProbes = false;
	bool showProbeTrails = false;
	bool showDebugGraphics = false;
	bool showStarStalks = true;
};

#endif // RENDERSYSTEM_H
