// SetupUI.cpp
#include "SetupUI.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace
{
	const sf::Color HEADING(150, 190, 235);
	const sf::Color LABEL(132, 146, 164);
	const sf::Color HELP(92, 104, 120);
	const sf::Color VALUE(228, 232, 238);
	const sf::Color TRACK(34, 44, 58);
	const sf::Color FILL(86, 132, 190);
	const sf::Color KNOB(206, 224, 244);
	const sf::Color PANEL_BG(10, 13, 18, 242);
	const sf::Color PANEL_EDGE(70, 92, 120);
	const sf::Color SEG_OFF(24, 31, 42);
	const sf::Color SEG_ON(52, 84, 126);

	const float PANEL_W = 820.0f;
	const float PANEL_H = 720.0f;
	const float PAD = 36.0f;
	const float TRACK_X = 300.0f;   // relative to panel left
	const float TRACK_W = 340.0f;
	const float ROW_H = 56.0f;

	std::string format(float v, int decimals, const std::string &suffix)
	{
		std::ostringstream ss;
		if (decimals <= 0)
		{
			// Thousands separators, because a fleet cap of 250000 is unreadable.
			long long n = static_cast<long long>(std::llround(v));
			std::string digits = std::to_string(n);
			std::string out;
			int count = 0;
			for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i)
			{
				out.insert(out.begin(), digits[static_cast<size_t>(i)]);
				if (++count % 3 == 0 && i > 0)
					out.insert(out.begin(), ',');
			}
			ss << out;
		}
		else
		{
			ss << std::fixed << std::setprecision(decimals) << v;
		}
		ss << suffix;
		return ss.str();
	}
} // namespace

SetupUI::SetupUI()
{
	sliders.resize(SliderCount);

	sliders[SearchRadius] = {"Search radius",
							 "How far a probe looks for its next target. Too small and it strands; too large and it never settles.",
							 " pc", 1.0f, 30.0f, 8.0f, 1, {}, {}};
	sliders[ReplicationLimit] = {"Replication limit",
								 "Copies each probe makes before shutting down. This is the exponent on the whole simulation.",
								 "", 0.0f, 10.0f, 3.0f, 0, {}, {}};
	sliders[ProbeSpeed] = {"Probe speed",
						   "Parsecs covered per tick. Faster reaches more, but replicates just as often on the way.",
						   " pc/tick", 0.05f, 2.0f, 0.25f, 2, {}, {}};
	sliders[FleetCap] = {"Fleet cap",
						 "Safety ceiling. Replication is free, so without this the fleet grows until the machine gives up.",
						 "", 0.0f, 0.0f, 250000.0f, 0,
						 {5000.f, 10000.f, 25000.f, 50000.f, 100000.f, 250000.f, 500000.f, 1000000.f}, {}};
	sliders[GalaxySize] = {"Stars loaded",
						   "Size of the catalogue, nearest first. Changing this reloads the data, which takes a moment.",
						   "", 0.0f, 0.0f, 50000.0f, 0,
						   {500.f, 1000.f, 2500.f, 5000.f, 10000.f, 25000.f, 50000.f}, {}};
	sliders[ViewTilt] = {"View tilt",
						 "90 looks straight down and hides height entirely; lower leans back so the stalks become readable.",
						 " deg", 30.0f, 90.0f, 75.0f, 0, {}, {}};
	sliders[ViewDepth] = {"View depth",
						  "How far above and below the plane the view reaches. Larger shows more, and more clutter.",
						  " pc", 2.0f, 40.0f, 8.0f, 0, {}, {}};

	spriteStyle.label = "Star sprite";
	spriteStyle.help = "Purely cosmetic. F5 also cycles these while running.";
	spriteStyle.options = {"Soft glow", "Core + halo", "Spikes", "Bloom ring"};
	spriteStyle.selected = 1;

	preset.label = "Expedition profile";
	preset.help = "A starting point. Move any slider and it becomes Custom.";
	preset.options = {"Survey", "Swarm", "Scout", "Custom"};
	preset.selected = 3;
}

int SetupUI::intValue(SliderId id) const
{
	return static_cast<int>(std::llround(sliders[id].value));
}

void SetupUI::setValue(SliderId id, float v)
{
	Slider &s = sliders[id];
	if (!s.allowed.empty())
	{
		float best = s.allowed.front();
		float bestDistance = std::fabs(v - best);
		for (float candidate : s.allowed)
		{
			const float d = std::fabs(v - candidate);
			if (d < bestDistance)
			{
				bestDistance = d;
				best = candidate;
			}
		}
		s.value = best;
		return;
	}
	s.value = std::max(s.minValue, std::min(s.maxValue, v));
}

void SetupUI::setSpriteChoice(int index)
{
	if (index >= 0 && index < static_cast<int>(spriteStyle.options.size()))
		spriteStyle.selected = index;
}

void SetupUI::applyPreset(int index)
{
	preset.selected = index;
	switch (index)
	{
	case 0: // Survey -- short hops, barely replicates. Slow but tidy.
		sliders[SearchRadius].value = 5.0f;
		sliders[ReplicationLimit].value = 1.0f;
		sliders[ProbeSpeed].value = 0.15f;
		break;
	case 1: // Swarm -- replicate hard, look no further than the doorstep.
		sliders[SearchRadius].value = 4.0f;
		sliders[ReplicationLimit].value = 5.0f;
		sliders[ProbeSpeed].value = 0.40f;
		break;
	case 2: // Scout -- long range, few children. Far frontier, thin coverage.
		sliders[SearchRadius].value = 20.0f;
		sliders[ReplicationLimit].value = 1.0f;
		sliders[ProbeSpeed].value = 0.80f;
		break;
	default:
		break;
	}
}

void SetupUI::layout(const sf::Vector2u &windowSize)
{
	panel = sf::FloatRect((windowSize.x - PANEL_W) * 0.5f, (windowSize.y - PANEL_H) * 0.5f, PANEL_W, PANEL_H);

	float y = panel.top + 118.0f;

	// preset row
	preset.boxes.clear();
	for (size_t i = 0; i < preset.options.size(); ++i)
	{
		preset.boxes.push_back(sf::FloatRect(panel.left + TRACK_X + i * 92.0f, y - 4.0f, 86.0f, 26.0f));
	}
	y += ROW_H;

	for (auto &s : sliders)
	{
		s.track = sf::FloatRect(panel.left + TRACK_X, y + 4.0f, TRACK_W, 6.0f);
		y += ROW_H;
	}

	spriteStyle.boxes.clear();
	for (size_t i = 0; i < spriteStyle.options.size(); ++i)
	{
		spriteStyle.boxes.push_back(sf::FloatRect(panel.left + TRACK_X + i * 92.0f, y - 4.0f, 86.0f, 26.0f));
	}

	launchButton = sf::FloatRect(panel.left + PANEL_W - PAD - 210.0f, panel.top + PANEL_H - 74.0f, 210.0f, 44.0f);
}

void SetupUI::setFromTrack(Slider &s, float mouseX)
{
	const float t = std::max(0.0f, std::min(1.0f, (mouseX - s.track.left) / s.track.width));
	if (!s.allowed.empty())
	{
		const size_t idx = static_cast<size_t>(std::llround(t * (s.allowed.size() - 1)));
		s.value = s.allowed[std::min(idx, s.allowed.size() - 1)];
		return;
	}
	float v = s.minValue + t * (s.maxValue - s.minValue);
	if (s.decimals <= 0)
		v = std::round(v);
	else
	{
		const float scale = std::pow(10.0f, static_cast<float>(s.decimals));
		v = std::round(v * scale) / scale;
	}
	s.value = v;
}

bool SetupUI::onMousePressed(const sf::Vector2f &p)
{
	for (size_t i = 0; i < preset.boxes.size(); ++i)
	{
		if (preset.boxes[i].contains(p))
		{
			applyPreset(static_cast<int>(i));
			return true;
		}
	}
	for (size_t i = 0; i < spriteStyle.boxes.size(); ++i)
	{
		if (spriteStyle.boxes[i].contains(p))
		{
			spriteStyle.selected = static_cast<int>(i);
			return true;
		}
	}
	for (size_t i = 0; i < sliders.size(); ++i)
	{
		// Generous vertical hit area -- a 6px track is unfair to aim at.
		sf::FloatRect hit = sliders[i].track;
		hit.top -= 14.0f;
		hit.height += 28.0f;
		hit.left -= 8.0f;
		hit.width += 16.0f;
		if (hit.contains(p))
		{
			draggingSlider = static_cast<int>(i);
			setFromTrack(sliders[i], p.x);
			preset.selected = 3; // Custom
			return true;
		}
	}
	return false;
}

bool SetupUI::onMouseMoved(const sf::Vector2f &p)
{
	if (draggingSlider < 0)
		return false;
	setFromTrack(sliders[static_cast<size_t>(draggingSlider)], p.x);
	return true;
}

void SetupUI::onMouseReleased() { draggingSlider = -1; }

bool SetupUI::launchClicked(const sf::Vector2f &p) const { return launchButton.contains(p); }

void SetupUI::draw(sf::RenderWindow &window, const sf::Font &font, const std::string &reachSummary) const
{
	const sf::Vector2f size(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));

	sf::RectangleShape veil(size);
	veil.setFillColor(sf::Color(4, 5, 8, 175));
	window.draw(veil);

	sf::RectangleShape bg(sf::Vector2f(panel.width, panel.height));
	bg.setPosition(panel.left, panel.top);
	bg.setFillColor(PANEL_BG);
	bg.setOutlineThickness(1.0f);
	bg.setOutlineColor(PANEL_EDGE);
	window.draw(bg);

	auto text = [&](const std::string &s, float x, float y, unsigned sz, const sf::Color &c) {
		sf::Text t(s, font, sz);
		t.setPosition(std::floor(x), std::floor(y));
		t.setFillColor(c);
		window.draw(t);
	};
	auto segments = [&](const Segmented &seg, float rowY) {
		text(seg.label, panel.left + PAD, rowY, 16, LABEL);
		text(seg.help, panel.left + PAD, rowY + 19.0f, 12, HELP);
		for (size_t i = 0; i < seg.boxes.size(); ++i)
		{
			sf::RectangleShape box(sf::Vector2f(seg.boxes[i].width, seg.boxes[i].height));
			box.setPosition(seg.boxes[i].left, seg.boxes[i].top);
			box.setFillColor(static_cast<int>(i) == seg.selected ? SEG_ON : SEG_OFF);
			box.setOutlineThickness(1.0f);
			box.setOutlineColor(static_cast<int>(i) == seg.selected ? HEADING : PANEL_EDGE);
			window.draw(box);
			text(seg.options[i], seg.boxes[i].left + 9.0f, seg.boxes[i].top + 4.0f, 13,
				 static_cast<int>(i) == seg.selected ? VALUE : LABEL);
		}
	};

	text("BEGIN EXPEDITION", panel.left + PAD, panel.top + 30.0f, 28, HEADING);
	text("Mouse sets the controls. Arrow keys still pan the map behind.",
		 panel.left + PAD, panel.top + 68.0f, 13, HELP);

	segments(preset, panel.top + 114.0f);

	for (const auto &s : sliders)
	{
		const float rowY = s.track.top - 8.0f;
		text(s.label, panel.left + PAD, rowY, 16, LABEL);
		text(s.help, panel.left + PAD, rowY + 19.0f, 12, HELP);

		sf::RectangleShape track(sf::Vector2f(s.track.width, s.track.height));
		track.setPosition(s.track.left, s.track.top);
		track.setFillColor(TRACK);
		window.draw(track);

		float t = 0.0f;
		if (!s.allowed.empty())
		{
			const auto it = std::find(s.allowed.begin(), s.allowed.end(), s.value);
			const size_t idx = (it == s.allowed.end()) ? 0 : static_cast<size_t>(it - s.allowed.begin());
			t = (s.allowed.size() > 1) ? static_cast<float>(idx) / (s.allowed.size() - 1) : 0.0f;
		}
		else if (s.maxValue > s.minValue)
		{
			t = (s.value - s.minValue) / (s.maxValue - s.minValue);
		}

		sf::RectangleShape filled(sf::Vector2f(s.track.width * t, s.track.height));
		filled.setPosition(s.track.left, s.track.top);
		filled.setFillColor(FILL);
		window.draw(filled);

		sf::CircleShape knob(7.0f);
		knob.setOrigin(7.0f, 7.0f);
		knob.setPosition(s.track.left + s.track.width * t, s.track.top + s.track.height * 0.5f);
		knob.setFillColor(KNOB);
		window.draw(knob);

		text(format(s.value, s.decimals, s.suffix), s.track.left + s.track.width + 24.0f, s.track.top - 10.0f, 16, VALUE);
	}

	segments(spriteStyle, spriteStyle.boxes.empty() ? panel.top : spriteStyle.boxes[0].top + 4.0f);

	sf::RectangleShape rule(sf::Vector2f(panel.width - 2.0f * PAD, 1.0f));
	rule.setPosition(panel.left + PAD, panel.top + panel.height - 96.0f);
	rule.setFillColor(sf::Color(48, 62, 82));
	window.draw(rule);

	text(reachSummary, panel.left + PAD, panel.top + panel.height - 66.0f, 14, LABEL);

	sf::RectangleShape button(sf::Vector2f(launchButton.width, launchButton.height));
	button.setPosition(launchButton.left, launchButton.top);
	button.setFillColor(SEG_ON);
	button.setOutlineThickness(1.0f);
	button.setOutlineColor(HEADING);
	window.draw(button);
	text("LAUNCH", launchButton.left + 62.0f, launchButton.top + 12.0f, 18, VALUE);
	text("or press Enter", launchButton.left - 118.0f, launchButton.top + 15.0f, 13, HELP);
}
