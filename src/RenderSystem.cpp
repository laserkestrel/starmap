// RenderSystem.cpp
#include "RenderSystem.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

namespace
{
	// Scales a colour's brightness while preserving its hue.
	//
	// The halo and core highlight used to be built by adding or subtracting a flat
	// 50 from each channel. That is fine for a bright star, but most stars in the
	// catalogue are faint and have already been scaled towards black by magnitude,
	// so adding a constant to near-zero channels produced a neutral grey. Multiplying
	// keeps the ratio between channels, and therefore the hue, at any brightness.
	sf::Color scaleColour(const sf::Color &colour, float factor)
	{
		const auto scale = [factor](sf::Uint8 channel) {
			const float scaled = static_cast<float>(channel) * factor;
			return static_cast<sf::Uint8>(std::max(0.0f, std::min(255.0f, scaled)));
		};
		return sf::Color(scale(colour.r), scale(colour.g), scale(colour.b), colour.a);
	}

	const float HALO_BRIGHTNESS = 0.55f;      // outer 3.5px ring, dimmer than the star
	const float HIGHLIGHT_BRIGHTNESS = 1.35f; // inner 2.5px core, brighter than the star
	const float STALK_BRIGHTNESS = 0.34f;     // the line down to the plane

	const float GRID_SPACING_PARSECS = 5.0f;
	const sf::Color GRID_COLOUR(20, 27, 38);
	const sf::Color GRID_AXIS_COLOUR(40, 54, 72);
	const sf::Color PLANE_FOOT_COLOUR(46, 56, 72);

	const float CULL_MARGIN_PIXELS = 8.0f;
} // namespace

RenderSystem::RenderSystem(sf::RenderWindow &window, const Projection &projection)
	: renderWindow(window), projection(projection)
{
	if (!font.loadFromFile("./content/Frontier.ttf"))
	{
		std::cerr << "Error loading font file.\n";
	}
	fpsCounter.setFont(font);
	fpsCounter.setCharacterSize(20);
	fpsCounter.setFillColor(sf::Color::White);
	fpsCounter.setPosition(10.f, 10.f);

	summaryText.setFont(font);
	summaryText.setCharacterSize(20);
	summaryText.setFillColor(sf::Color::White);
	summaryText.setPosition(20.f, 50.f);
}

void RenderSystem::toggleTextLabelsStars() { showTextLabelsStars = !showTextLabelsStars; }
void RenderSystem::toggleTextLabelsProbes() { showTextLabelsProbes = !showTextLabelsProbes; }
void RenderSystem::toggleProbeTrails() { showProbeTrails = !showProbeTrails; }
void RenderSystem::toggleDebugGraphics() { showDebugGraphics = !showDebugGraphics; }
void RenderSystem::toggleStarStalks() { showStarStalks = !showStarStalks; }

void RenderSystem::initializeStarsTexture(const std::vector<Star> &stars)
{
	const unsigned int width = renderWindow.getSize().x;
	const unsigned int height = renderWindow.getSize().y;

	sf::RenderTexture renderTexture;
	renderTexture.create(width, height);
	renderTexture.clear(sf::Color(5, 6, 9));

	// --- reference grid on the z = 0 plane -------------------------------------
	// Without it the tilt is invisible and the stalks have nothing to sit on.
	{
		const float halfSpanX = (static_cast<float>(width) * 0.5f) / projection.getPixelsPerParsec();
		const float sinTilt = std::max(0.05f, std::sin(projection.getTiltDegrees() * 3.14159265f / 180.0f));
		const float halfSpanY = (static_cast<float>(height) * 0.5f) / (projection.getPixelsPerParsec() * sinTilt);

		const float startX = std::floor(-halfSpanX / GRID_SPACING_PARSECS) * GRID_SPACING_PARSECS;
		const float startY = std::floor(-halfSpanY / GRID_SPACING_PARSECS) * GRID_SPACING_PARSECS;

		sf::VertexArray grid(sf::Lines);
		for (float gx = startX; gx <= halfSpanX; gx += GRID_SPACING_PARSECS)
		{
			const sf::Color c = (std::abs(gx) < 0.01f) ? GRID_AXIS_COLOUR : GRID_COLOUR;
			grid.append(sf::Vertex(projection.planeFoot(gx, -halfSpanY), c));
			grid.append(sf::Vertex(projection.planeFoot(gx, halfSpanY), c));
		}
		for (float gy = startY; gy <= halfSpanY; gy += GRID_SPACING_PARSECS)
		{
			const sf::Color c = (std::abs(gy) < 0.01f) ? GRID_AXIS_COLOUR : GRID_COLOUR;
			grid.append(sf::Vertex(projection.planeFoot(-halfSpanX, gy), c));
			grid.append(sf::Vertex(projection.planeFoot(halfSpanX, gy), c));
		}
		renderTexture.draw(grid);
	}

	// --- decide what is worth drawing, and in what order ------------------------
	// Painter's algorithm: far stars first so near ones overlap them.
	std::vector<size_t> visible;
	visible.reserve(stars.size());
	for (size_t i = 0; i < stars.size(); ++i)
	{
		if (!projection.withinViewDepth(stars[i].getWorldZ()))
		{
			continue; // outside the slab this view covers
		}
		const sf::Vector2f p = projection.project(stars[i].getWorldX(), stars[i].getWorldY(), stars[i].getWorldZ());
		const sf::Vector2f foot = projection.planeFoot(stars[i].getWorldX(), stars[i].getWorldY());
		// Keep a star if either it or the foot of its stalk is on screen.
		const bool starOn = p.x >= -CULL_MARGIN_PIXELS && p.x <= static_cast<float>(width) + CULL_MARGIN_PIXELS &&
							p.y >= -CULL_MARGIN_PIXELS && p.y <= static_cast<float>(height) + CULL_MARGIN_PIXELS;
		const bool footOn = foot.y >= -CULL_MARGIN_PIXELS && foot.y <= static_cast<float>(height) + CULL_MARGIN_PIXELS &&
							foot.x >= -CULL_MARGIN_PIXELS && foot.x <= static_cast<float>(width) + CULL_MARGIN_PIXELS;
		if (starOn || footOn)
		{
			visible.push_back(i);
		}
	}
	std::sort(visible.begin(), visible.end(), [&stars, this](size_t a, size_t b) {
		return projection.depth(stars[a].getWorldY(), stars[a].getWorldZ()) >
			   projection.depth(stars[b].getWorldY(), stars[b].getWorldZ());
	});

	// --- stalks, batched into a single draw ------------------------------------
	if (showStarStalks)
	{
		sf::VertexArray stalks(sf::Lines);
		stalks.resize(0);
		for (size_t idx : visible)
		{
			const Star &star = stars[idx];
			const sf::Vector2f top = projection.project(star.getWorldX(), star.getWorldY(), star.getWorldZ());
			const sf::Vector2f foot = projection.planeFoot(star.getWorldX(), star.getWorldY());
			const sf::Color c = scaleColour(star.getColour(), STALK_BRIGHTNESS);
			stalks.append(sf::Vertex(foot, c));
			stalks.append(sf::Vertex(top, c));
		}
		renderTexture.draw(stalks);

		// a small mark where each stalk meets the plane, so height is readable
		sf::VertexArray feet(sf::Points);
		for (size_t idx : visible)
		{
			const Star &star = stars[idx];
			feet.append(sf::Vertex(projection.planeFoot(star.getWorldX(), star.getWorldY()), PLANE_FOOT_COLOUR));
		}
		renderTexture.draw(feet);
	}

	// --- the stars themselves ---------------------------------------------------
	for (size_t idx : visible)
	{
		const Star &star = stars[idx];
		const sf::Vector2f p = projection.project(star.getWorldX(), star.getWorldY(), star.getWorldZ());

		sf::CircleShape baseShape(3.5f);
		baseShape.setOrigin(3.5f, 3.5f);
		baseShape.setPosition(p);
		baseShape.setFillColor(scaleColour(star.getColour(), HALO_BRIGHTNESS));
		renderTexture.draw(baseShape);

		sf::CircleShape coreShape(3.0f);
		coreShape.setOrigin(3.0f, 3.0f);
		coreShape.setPosition(p);
		coreShape.setFillColor(star.getColour());
		renderTexture.draw(coreShape);

		sf::CircleShape centerShape(2.5f);
		centerShape.setOrigin(2.5f, 2.5f);
		centerShape.setPosition(p);
		centerShape.setFillColor(scaleColour(star.getColour(), HIGHLIGHT_BRIGHTNESS));
		renderTexture.draw(centerShape);

		if (showTextLabelsStars && !star.getName().empty())
		{
			sf::Text labelText(star.getName(), font, 14);
			labelText.setPosition(p.x + 8.0f, p.y - 18.0f);
			renderTexture.draw(labelText);
		}
	}

	renderTexture.display(); // don't remove: without it the texture is undefined
	starsTexture = renderTexture.getTexture();
}

void RenderSystem::renderProbes(const std::vector<Probe> &probes)
{
	// Trails first, so probe markers sit on top of them.
	if (showProbeTrails)
	{
		sf::VertexArray trails(sf::Lines);
		for (const auto &probe : probes)
		{
			const auto &visited = probe.getVisitedStarSystems();
			const sf::Color pathColor = probe.getTrailColor();
			for (size_t i = 1; i < visited.size(); ++i)
			{
				if (!visited[i].visitedByProbe || !visited[i - 1].visitedByProbe)
					continue;
				const auto &a = visited[i - 1].coordinates;
				const auto &b = visited[i].coordinates;
				trails.append(sf::Vertex(projection.project(a.x, a.y, a.z), pathColor));
				trails.append(sf::Vertex(projection.project(b.x, b.y, b.z), pathColor));
			}
		}
		renderWindow.draw(trails);
	}

	// One batched draw for every probe marker, rather than a CircleShape each.
	// The whole point of the simulation is exponential probe growth, so a draw
	// call per probe scaled exactly the wrong way.
	sf::VertexArray markers(sf::Points);
	markers.resize(0);
	for (const auto &probe : probes)
	{
		markers.append(sf::Vertex(projection.project(probe.getWorldX(), probe.getWorldY(), probe.getWorldZ()),
								  sf::Color(173, 216, 230)));
	}
	renderWindow.draw(markers);

	if (showTextLabelsProbes)
	{
		for (const auto &probe : probes)
		{
			const sf::Vector2f p = projection.project(probe.getWorldX(), probe.getWorldY(), probe.getWorldZ());
			sf::Text labelText(probe.getProbeName(), font, 10);
			labelText.setPosition(p.x - 10.0f, p.y - 10.0f);
			labelText.setFillColor(probe.getTrailColor());
			renderWindow.draw(labelText);
		}
	}
}

void RenderSystem::renderSummaryText(const std::string &summary)
{
	summaryText.setString(summary);
	summaryText.setPosition(10, 10);
	renderWindow.draw(summaryText);
}

void RenderSystem::renderParameterList(const std::vector<std::pair<std::string, std::string>> &params, int focusedIndex, bool showCaret)
{
	const float startX = 20.f;
	const float y = 50.f;
	const float lineHeight = 26.f;

	for (size_t i = 0; i < params.size(); ++i)
	{
		sf::Text keyText(params[i].first + ": ", font, 20);
		keyText.setPosition(startX, y + i * lineHeight);
		keyText.setFillColor(sf::Color::White);

		sf::Text valueText(params[i].second, font, 20);
		valueText.setPosition(startX + 300.f, y + i * lineHeight);
		valueText.setFillColor(sf::Color::White);

		if (static_cast<int>(i) == focusedIndex)
		{
			sf::RectangleShape bg(sf::Vector2f(700.f, lineHeight));
			bg.setPosition(startX - 6.f, y + i * lineHeight - 2.f);
			bg.setFillColor(sf::Color(64, 64, 64, 160));
			renderWindow.draw(bg);
			if (showCaret)
			{
				const float caretX = valueText.getPosition().x + valueText.getLocalBounds().width + 4.f;
				const float caretY = valueText.getPosition().y;
				sf::RectangleShape caret(sf::Vector2f(2.f, static_cast<float>(valueText.getCharacterSize())));
				caret.setPosition(caretX, caretY);
				caret.setFillColor(sf::Color::White);
				renderWindow.draw(caret);
			}
		}

		renderWindow.draw(keyText);
		renderWindow.draw(valueText);
	}

	sf::Text instr("Tab: Next  Shift+Tab: Prev  Type numbers or use Up/Down  Enter: Start", font, 16);
	instr.setPosition(startX, y + params.size() * lineHeight + 8.f);
	instr.setFillColor(sf::Color(200, 200, 200));
	renderWindow.draw(instr);
}

void RenderSystem::calculateAndDisplayFPS()
{
	if (showDebugGraphics)
	{
		const sf::Time elapsed = fpsClock.restart();
		const float fps = (elapsed.asSeconds() > 0.f) ? (1.0f / elapsed.asSeconds()) : 0.f;

		std::ostringstream ss;
		ss << "FPS: " << static_cast<int>(fps);
		fpsCounter.setString(ss.str());
		renderWindow.draw(fpsCounter);
	}
}

void RenderSystem::renderQuadtree(sf::RenderWindow &window, const GalaxyQuadTreeNode *node)
{
	if (node == nullptr || !showDebugGraphics)
	{
		return;
	}

	// The tree's cells are world-axis-aligned rectangles on the z = 0 plane, and
	// the projection scales x and y independently, so they stay rectangles on
	// screen -- just vertically squashed by the tilt.
	const sf::Vector2f topLeft = projection.planeFoot(node->boundary.left, node->boundary.top);
	const sf::Vector2f bottomRight = projection.planeFoot(node->boundary.left + node->boundary.width,
														  node->boundary.top + node->boundary.height);

	sf::RectangleShape nodeRect;
	nodeRect.setPosition(topLeft);
	nodeRect.setSize(sf::Vector2f(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y));
	nodeRect.setFillColor(sf::Color::Transparent);
	nodeRect.setOutlineThickness(0.5f);
	nodeRect.setOutlineColor(sf::Color(55, 55, 55, 128));
	window.draw(nodeRect);

	if (!node->isLeaf)
	{
		for (int i = 0; i < 4; ++i)
		{
			renderQuadtree(window, node->getChild(i));
		}
	}
}
