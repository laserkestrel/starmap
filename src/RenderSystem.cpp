// RenderSystem.cpp
#include "RenderSystem.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
	const float STALK_BRIGHTNESS = 0.20f; // the line down to the plane

	// Brighter stars are drawn larger as well as brighter. Size alone carries a
	// surprising amount of a starfield's readability -- every star being the same
	// 3.5px was why the old field looked flat.
	const float STAR_MIN_SIZE_PIXELS = 8.0f;
	const float STAR_MAX_SIZE_PIXELS = 30.0f;
	const float PROBE_SIZE_PIXELS = 10.0f;

	// A sprite spreads its energy across a soft falloff, so its peak is far below
	// that of the solid circles this replaced. This gain restores the punch -- but
	// it is capped per star so no channel clips, which would desaturate the colour
	// towards white and undo the whole point of deriving it from the star's
	// temperature. Faint stars get the full lift; already-bright ones get less.
	const float STAR_GAIN = 1.7f;

	sf::Color boostPreservingHue(const sf::Color &colour, float gain);

	const float GRID_SPACING_PARSECS = 5.0f;
	const sf::Color GRID_COLOUR(20, 27, 38);
	const sf::Color GRID_AXIS_COLOUR(42, 56, 74);
	const sf::Color BACKGROUND_COLOUR(5, 6, 9);

	const float CULL_MARGIN_PIXELS = 32.0f;

	sf::Color scaleColour(const sf::Color &colour, float factor)
	{
		const auto scale = [factor](sf::Uint8 channel) {
			const float scaled = static_cast<float>(channel) * factor;
			return static_cast<sf::Uint8>(std::max(0.0f, std::min(255.0f, scaled)));
		};
		return sf::Color(scale(colour.r), scale(colour.g), scale(colour.b), colour.a);
	}

	sf::Color boostPreservingHue(const sf::Color &colour, float gain)
	{
		const float peak = static_cast<float>(std::max(colour.r, std::max(colour.g, colour.b)));
		if (peak <= 0.0f)
		{
			return colour;
		}
		return scaleColour(colour, std::min(gain, 255.0f / peak));
	}
} // namespace

RenderSystem::RenderSystem(sf::RenderWindow &window, const Projection &projection, StarSpriteStyle spriteStyle)
	: renderWindow(window), projection(projection), spriteStyle(spriteStyle)
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

	hudText.setFont(font);
	hudText.setCharacterSize(15);
	hudText.setFillColor(sf::Color(140, 152, 168));

	starSprite = makeStarSprite(spriteStyle);
}

void RenderSystem::toggleTextLabelsStars() { showTextLabelsStars = !showTextLabelsStars; }
void RenderSystem::toggleTextLabelsProbes() { showTextLabelsProbes = !showTextLabelsProbes; }
void RenderSystem::toggleProbeTrails() { showProbeTrails = !showProbeTrails; }
void RenderSystem::toggleDebugGraphics() { showDebugGraphics = !showDebugGraphics; }
void RenderSystem::toggleStarStalks() { showStarStalks = !showStarStalks; }

void RenderSystem::reloadGraphicsResources()
{
	if (!font.loadFromFile("./content/Frontier.ttf"))
	{
		std::cerr << "Error reloading font file.\n";
	}
	fpsCounter.setFont(font);
	summaryText.setFont(font);
	hudText.setFont(font);
	starSprite = makeStarSprite(spriteStyle);
}

void RenderSystem::setSpriteStyle(StarSpriteStyle style)
{
	spriteStyle = style;
	starSprite = makeStarSprite(spriteStyle);
}

StarSpriteStyle RenderSystem::cycleSpriteStyle()
{
	const int next = (static_cast<int>(spriteStyle) + 1) % static_cast<int>(StarSpriteStyle::Count);
	setSpriteStyle(static_cast<StarSpriteStyle>(next));
	return spriteStyle;
}

void RenderSystem::appendSpriteQuad(sf::VertexArray &target, const sf::Vector2f &centre, float sizePixels, const sf::Color &colour) const
{
	const float half = sizePixels * 0.5f;
	const float tex = static_cast<float>(starSprite.getSize().x);

	target.append(sf::Vertex(sf::Vector2f(centre.x - half, centre.y - half), colour, sf::Vector2f(0.f, 0.f)));
	target.append(sf::Vertex(sf::Vector2f(centre.x + half, centre.y - half), colour, sf::Vector2f(tex, 0.f)));
	target.append(sf::Vertex(sf::Vector2f(centre.x + half, centre.y + half), colour, sf::Vector2f(tex, tex)));
	target.append(sf::Vertex(sf::Vector2f(centre.x - half, centre.y + half), colour, sf::Vector2f(0.f, tex)));
}

void RenderSystem::renderStarfield(const std::vector<Star> &stars, const GalaxyQuadTree &quadTree)
{
	const unsigned int width = renderWindow.getSize().x;
	const unsigned int height = renderWindow.getSize().y;

	renderWindow.clear(BACKGROUND_COLOUR);

	// --- reference grid on the z = 0 plane -------------------------------------
	gridLines.clear();
	{
		const sf::FloatRect view = projection.visibleWorldBounds(width, height, GRID_SPACING_PARSECS);
		const float startX = std::floor(view.left / GRID_SPACING_PARSECS) * GRID_SPACING_PARSECS;
		const float endX = view.left + view.width;
		const float startY = std::floor(view.top / GRID_SPACING_PARSECS) * GRID_SPACING_PARSECS;
		const float endY = view.top + view.height;

		// Guard against a pathological zoom producing millions of grid lines.
		const int maxLines = 400;
		if ((endX - startX) / GRID_SPACING_PARSECS < maxLines && (endY - startY) / GRID_SPACING_PARSECS < maxLines)
		{
			for (float gx = startX; gx <= endX; gx += GRID_SPACING_PARSECS)
			{
				const sf::Color c = (std::abs(gx) < 0.01f) ? GRID_AXIS_COLOUR : GRID_COLOUR;
				gridLines.append(sf::Vertex(projection.planeFoot(gx, startY), c));
				gridLines.append(sf::Vertex(projection.planeFoot(gx, endY), c));
			}
			for (float gy = startY; gy <= endY; gy += GRID_SPACING_PARSECS)
			{
				const sf::Color c = (std::abs(gy) < 0.01f) ? GRID_AXIS_COLOUR : GRID_COLOUR;
				gridLines.append(sf::Vertex(projection.planeFoot(startX, gy), c));
				gridLines.append(sf::Vertex(projection.planeFoot(endX, gy), c));
			}
		}
	}
	renderWindow.draw(gridLines);

	// --- ask the tree for candidates, then cull precisely ------------------------
	visibleScratch.clear();
	quadTree.queryRange(projection.visibleWorldBounds(width, height), visibleScratch);

	visible.clear();
	visible.reserve(visibleScratch.size());
	for (size_t idx : visibleScratch)
	{
		const Star &star = stars[idx];
		if (!projection.withinViewDepth(star.getWorldZ()))
		{
			continue;
		}
		const sf::Vector2f p = projection.project(star.getWorldX(), star.getWorldY(), star.getWorldZ());
		const sf::Vector2f foot = projection.planeFoot(star.getWorldX(), star.getWorldY());
		const bool starOn = p.x >= -CULL_MARGIN_PIXELS && p.x <= width + CULL_MARGIN_PIXELS &&
							p.y >= -CULL_MARGIN_PIXELS && p.y <= height + CULL_MARGIN_PIXELS;
		const bool footOn = foot.x >= -CULL_MARGIN_PIXELS && foot.x <= width + CULL_MARGIN_PIXELS &&
							foot.y >= -CULL_MARGIN_PIXELS && foot.y <= height + CULL_MARGIN_PIXELS;
		if (starOn || footOn)
		{
			visible.push_back(idx);
		}
	}
	lastVisibleStarCount = visible.size();

	// Painter's algorithm: far stars first so near ones overlap them.
	std::sort(visible.begin(), visible.end(), [&stars, this](size_t a, size_t b) {
		return projection.depth(stars[a].getWorldY(), stars[a].getWorldZ()) >
			   projection.depth(stars[b].getWorldY(), stars[b].getWorldZ());
	});

	// --- stalks, one draw call ---------------------------------------------------
	stalkLines.clear();
	if (showStarStalks)
	{
		for (size_t idx : visible)
		{
			const Star &star = stars[idx];
			const sf::Vector2f top = projection.project(star.getWorldX(), star.getWorldY(), star.getWorldZ());
			const sf::Vector2f foot = projection.planeFoot(star.getWorldX(), star.getWorldY());
			const sf::Color c = scaleColour(star.getColour(), STALK_BRIGHTNESS);
			stalkLines.append(sf::Vertex(foot, c));
			stalkLines.append(sf::Vertex(top, c));
		}
		renderWindow.draw(stalkLines);
	}

	// --- stars, one draw call, additively blended --------------------------------
	// Additive is what makes the field read as light rather than paint: overlapping
	// stars sum, crowded regions bloom, and bright cores saturate to white.
	starQuads.clear();
	for (size_t idx : visible)
	{
		const Star &star = stars[idx];
		const sf::Vector2f p = projection.project(star.getWorldX(), star.getWorldY(), star.getWorldZ());
		const float t = std::max(0.0f, std::min(1.0f, (star.getDisplayBrightness() - 0.35f) / 0.65f));
		const float size = STAR_MIN_SIZE_PIXELS + (STAR_MAX_SIZE_PIXELS - STAR_MIN_SIZE_PIXELS) * t;
		appendSpriteQuad(starQuads, p, size, boostPreservingHue(star.getColour(), STAR_GAIN));
	}
	{
		sf::RenderStates states;
		states.blendMode = sf::BlendAdd;
		states.texture = &starSprite;
		renderWindow.draw(starQuads, states);
	}

	// --- labels: the brightest few ----------------------------------------------
	// Labelling everything is unreadable once a few hundred stars are on screen,
	// and each label is its own draw call since sf::Text cannot be batched. Ranking
	// by brightness and keeping the top N means the prominent stars stay named at
	// any zoom and the cost is bounded no matter how far out you go.
	if (showTextLabelsStars && labelMaxVisible > 0)
	{
		labelCandidates.clear();
		for (size_t idx : visible)
		{
			if (!stars[idx].getName().empty())
			{
				labelCandidates.push_back(idx);
			}
		}

		if (labelCandidates.size() > labelMaxVisible)
		{
			std::nth_element(labelCandidates.begin(), labelCandidates.begin() + labelMaxVisible,
							 labelCandidates.end(), [&stars](size_t a, size_t b) {
								 return stars[a].getDisplayBrightness() > stars[b].getDisplayBrightness();
							 });
			labelCandidates.resize(labelMaxVisible);
		}

		for (size_t idx : labelCandidates)
		{
			const Star &star = stars[idx];
			const sf::Vector2f p = projection.project(star.getWorldX(), star.getWorldY(), star.getWorldZ());
			sf::Text labelText(star.getName(), font, 14);
			labelText.setPosition(p.x + 9.0f, p.y - 18.0f);
			labelText.setFillColor(sf::Color(150, 190, 235));
			renderWindow.draw(labelText);
		}
	}
}

void RenderSystem::renderProbes(const std::vector<Probe> &probes)
{
	if (showProbeTrails)
	{
		trailLines.clear();
		for (const auto &probe : probes)
		{
			// The trail is now only where this probe itself went, so every
			// consecutive pair is a real leg of its journey.
			const auto &visited = probe.getTrail();
			const sf::Color pathColor = probe.getTrailColor();
			for (size_t i = 1; i < visited.size(); ++i)
			{
				const auto &a = visited[i - 1].coordinates;
				const auto &b = visited[i].coordinates;
				trailLines.append(sf::Vertex(projection.project(a.x, a.y, a.z), pathColor));
				trailLines.append(sf::Vertex(projection.project(b.x, b.y, b.z), pathColor));
			}
		}
		renderWindow.draw(trailLines);
	}

	// Probes share the star sprite, so thousands of them are still one draw call.
	probeQuads.clear();
	for (const auto &probe : probes)
	{
		const sf::Vector2f p = projection.project(probe.getWorldX(), probe.getWorldY(), probe.getWorldZ());
		appendSpriteQuad(probeQuads, p, PROBE_SIZE_PIXELS, sf::Color(150, 205, 255));
	}
	{
		sf::RenderStates states;
		states.blendMode = sf::BlendAdd;
		states.texture = &starSprite;
		renderWindow.draw(probeQuads, states);
	}

	if (showTextLabelsProbes)
	{
		size_t drawn = 0;
		for (const auto &probe : probes)
		{
			if (drawn++ >= labelMaxVisible)
				break;
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

void RenderSystem::renderHud(const std::string &text)
{
	hudText.setString(text);
	hudText.setPosition(12.0f, static_cast<float>(renderWindow.getSize().y) - 26.0f);
	renderWindow.draw(hudText);
}


void RenderSystem::calculateAndDisplayFPS()
{
	if (showDebugGraphics)
	{
		const sf::Time elapsed = fpsClock.restart();
		const float fps = (elapsed.asSeconds() > 0.f) ? (1.0f / elapsed.asSeconds()) : 0.f;

		std::ostringstream ss;
		ss << "FPS: " << static_cast<int>(fps) << "   stars drawn: " << lastVisibleStarCount;
		fpsCounter.setString(ss.str());
		renderWindow.draw(fpsCounter);
	}
	else
	{
		fpsClock.restart();
	}
}

void RenderSystem::renderQuadtree(sf::RenderWindow &window, const GalaxyQuadTreeNode *node)
{
	if (node == nullptr || !showDebugGraphics)
	{
		return;
	}

	const sf::Vector2f topLeft = projection.planeFoot(node->boundary.left, node->boundary.top);
	const sf::Vector2f bottomRight = projection.planeFoot(node->boundary.left + node->boundary.width,
														  node->boundary.top + node->boundary.height);

	// Skip cells that are entirely off screen, and cells too small to see.
	const float w = bottomRight.x - topLeft.x;
	const float h = bottomRight.y - topLeft.y;
	if (w < 3.0f || h < 3.0f)
		return;
	if (bottomRight.x < 0 || bottomRight.y < 0 ||
		topLeft.x > static_cast<float>(window.getSize().x) || topLeft.y > static_cast<float>(window.getSize().y))
		return;

	sf::RectangleShape nodeRect;
	nodeRect.setPosition(topLeft);
	nodeRect.setSize(sf::Vector2f(w, h));
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

void RenderSystem::renderDebrief(const RunMetrics &metrics)
{
	const float width = static_cast<float>(renderWindow.getSize().x);
	const float height = static_cast<float>(renderWindow.getSize().y);

	const float panelW = 720.0f;
	const float panelH = 560.0f;
	const float x = (width - panelW) * 0.5f;
	const float y = (height - panelH) * 0.5f;

	// Dim the map so the panel reads, but leave it visible -- the result is the
	// picture behind the numbers.
	sf::RectangleShape veil(sf::Vector2f(width, height));
	veil.setFillColor(sf::Color(4, 5, 8, 190));
	renderWindow.draw(veil);

	sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
	panel.setPosition(x, y);
	panel.setFillColor(sf::Color(10, 13, 18, 240));
	panel.setOutlineThickness(1.0f);
	panel.setOutlineColor(sf::Color(70, 92, 120));
	renderWindow.draw(panel);

	const sf::Color heading(150, 190, 235);
	const sf::Color label(132, 146, 164);
	const sf::Color value(228, 232, 238);

	auto text = [&](const std::string &s, float px, float py, unsigned size, const sf::Color &c) {
		sf::Text t(s, font, size);
		t.setPosition(std::floor(px), std::floor(py));
		t.setFillColor(c);
		renderWindow.draw(t);
	};
	auto row = [&](const std::string &name, const std::string &val, float py) {
		text(name, x + 34.0f, py, 16, label);
		text(val, x + 330.0f, py, 16, value);
	};
	auto num = [](double v, int dp) {
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(dp) << v;
		return ss.str();
	};

	float py = y + 28.0f;
	text("EXPEDITION COMPLETE", x + 34.0f, py, 28, heading);
	py += 38.0f;
	text(runEndReasonText(metrics.endReason) + std::string("  -  ") +
			 std::to_string(metrics.ticks) + " ticks",
		 x + 34.0f, py, 15, label);

	py += 44.0f;
	text("REACH", x + 34.0f, py, 15, heading);
	py += 26.0f;
	row("systems reached", std::to_string(metrics.uniqueSystems) + " of " +
							  std::to_string(metrics.catalogueSize) +
							  "   (" + num(metrics.coveragePercent(), 2) + "%)", py);
	py += 24.0f;
	row("frontier", num(metrics.frontierParsecs, 1) + " pc from Sol", py);
	py += 24.0f;
	row("expansion rate", num(metrics.expansionRatePerThousandTicks(), 2) + " pc / 1000 ticks", py);

	py += 40.0f;
	text("EFFICIENCY", x + 34.0f, py, 15, heading);
	py += 26.0f;
	row("efficiency", num(metrics.efficiency(), 3) + "   (1.000 = nothing wasted)", py);
	py += 24.0f;
	row("wasted journeys", std::to_string(metrics.wastedJourneys()) + " of " +
							   std::to_string(metrics.arrivals) + " arrivals", py);
	py += 24.0f;
	row("cost per system", num(metrics.parsecsPerDiscovery(), 2) + " pc flown, " +
							   num(metrics.probesPerDiscovery(), 2) + " probes built", py);

	py += 40.0f;
	text("FLEET", x + 34.0f, py, 15, heading);
	py += 26.0f;
	row("built / peak / alive", std::to_string(metrics.probesBuilt) + "  /  " +
									std::to_string(metrics.peakPopulation) + "  /  " +
									std::to_string(metrics.probesAlive), py);
	py += 24.0f;
	row("hit replication limit", std::to_string(metrics.stoppedAtReplicationLimit), py);
	py += 24.0f;
	row("nothing within range", std::to_string(metrics.stoppedWithNothingInRange), py);

	py += 44.0f;
	sf::RectangleShape rule(sf::Vector2f(panelW - 68.0f, 1.0f));
	rule.setPosition(x + 34.0f, py);
	rule.setFillColor(sf::Color(48, 62, 82));
	renderWindow.draw(rule);

	py += 18.0f;
	text("SCORE", x + 34.0f, py + 10.0f, 15, heading);
	text(std::to_string(metrics.score()), x + 330.0f, py, 34, value);
	text("GRADE", x + panelW - 210.0f, py + 10.0f, 15, heading);
	text(metrics.grade(), x + panelW - 110.0f, py - 4.0f, 40, heading);

	text("arrows pan   wheel zooms   Esc closes", x + 34.0f, y + panelH - 30.0f, 14, label);
}
