// RenderSystem.cpp
#include "RenderSystem.h"
#include <algorithm>
#include <cmath>
#include <iostream>

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
			const auto &visited = probe.getVisitedStarSystems();
			const sf::Color pathColor = probe.getTrailColor();
			for (size_t i = 1; i < visited.size(); ++i)
			{
				if (!visited[i].visitedByProbe || !visited[i - 1].visitedByProbe)
					continue;
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
				sf::RectangleShape caret(sf::Vector2f(2.f, static_cast<float>(valueText.getCharacterSize())));
				caret.setPosition(caretX, valueText.getPosition().y);
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
