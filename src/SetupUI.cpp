// SetupUI.cpp
#include "SetupUI.h"
#include "TrailStyle.h"
#include "Genome.h"
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
	// Sized for the taller of the two tabs (Simulation), plus the tab bar. Splitting
	// display settings out is what stops this growing every time something is added.
	const float PANEL_H = 1010.0f;
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
						 "Safety ceiling, and a backstop rather than a rule -- with the economy on, scarcity should be what ends a run.",
						 "", 0.0f, 0.0f, 250000.0f, 0,
						 {5000.f, 10000.f, 25000.f, 50000.f, 100000.f, 250000.f, 500000.f, 1000000.f}, {}};
	sliders[GalaxySize] = {"Stars loaded",
						   "Size of the catalogue, nearest first. Changing this reloads the data, which takes a moment.",
						   "", 0.0f, 0.0f, 50000.0f, 0,
						   {500.f, 1000.f, 2500.f, 5000.f, 10000.f, 25000.f, 50000.f}, {}};
	sliders[SystemRichness] = {"System richness",
							   "Material in an average system. Abundance breeds a bigger fleet, and a bigger fleet wastes more journeys: doubling this halved efficiency in testing.",
							   " units", 25.0f, 400.0f, 100.0f, 0, {}, {}};
	sliders[ProbeBuildCost] = {"Probe build cost",
							   "What one copy costs, against the standard bill of materials. Dearer probes mean a smaller fleet that lives longer and treads on itself less.",
							   "x", 0.25f, 3.0f, 1.0f, 2, {}, {}};
	sliders[FuelBurn] = {"Fuel burn",
						 "Volatiles spent per parsec flown. This is what makes distance dangerous -- a probe that runs dry between stars is simply lost.",
						 " /pc", 0.25f, 6.0f, 1.5f, 2, {}, {}};
	sliders[MutationStrength] = {"Mutation",
								 "How far a child's traits may drift from its parent's, per generation. Zero means every probe is a copy of the founder; too high and inheritance stops meaning anything.",
								 "%", 0.0f, 30.0f, 8.0f, 0, {}, {}};
	sliders[ViewTilt] = {"View tilt",
						 "90 looks straight down and hides height entirely; lower leans back so the stalks become readable.",
						 " deg", 30.0f, 90.0f, 75.0f, 0, {}, {}, DisplayTab};
	sliders[ViewDepth] = {"View depth",
						  "How far above and below the plane the view reaches. Larger shows more, and more clutter.",
						  " pc", 2.0f, 40.0f, 8.0f, 0, {}, {}, DisplayTab};
	sliders[TrailFade] = {"Trail fade",
						  "How many ticks a trail takes to cool from white to dark, in Recency mode. Short values leave only the expansion front lit.",
						  " ticks", 50.0f, 3000.0f, 600.0f, 0, {}, {}, DisplayTab};

	spriteStyle.label = "Star sprite";
	spriteStyle.help = "Purely cosmetic. F5 also cycles these while running.";
	spriteStyle.options = {"Soft glow", "Core + halo", "Spikes", "Bloom ring"};
	spriteStyle.selected = 1;
	spriteStyle.tab = DisplayTab;

	trailMode.label = "Trail colouring";
	trailMode.help = "Recency fades a leg as it ages. Density burns brightest where the most probes have arrived, which is where the waste is. Lineage gives each family its own band of hue. Trait colours by the genome value chosen below. F6 cycles them live.";
	trailMode.options = {"Recency", "Density", "Lineage", "Trait"};
	trailMode.selected = 0;
	trailMode.tab = DisplayTab;

	trailPaletteControl.label = "Trail palette";
	trailPaletteControl.help = "The hue the heat ramp runs through, dark to white. Applies to both Recency and Density. F7 cycles these while running.";
	trailPaletteControl.options.clear();
	for (int i = 0; i < trailPaletteCount(); ++i)
	{
		trailPaletteControl.options.push_back(trailPalettes()[i].name);
	}
	trailPaletteControl.selected = 0;
	trailPaletteControl.tab = DisplayTab;
	// Eight options will not fit at the default width.
	trailPaletteControl.boxWidth = 54.0f;
	trailPaletteControl.boxSpacing = 58.0f;

	evolution.label = "Evolution";
	evolution.help = "On, a child inherits its parent's traits with a small random change and nothing else selects -- whichever values produce surviving descendants become common. The sliders above then set what the FIRST probe believes, not fleet-wide law.";
	evolution.options = {"Off", "On"};
	evolution.selected = 1;

	traitColour.label = "Colour trails by";
	traitColour.help = "Which trait the Trait trail mode shows. The map starts mottled and converges towards one colour if a strategy is taking over.";
	traitColour.options.clear();
	for (int i = 0; i < static_cast<int>(Trait::TraitCount); ++i)
		traitColour.options.push_back(traitInfo(static_cast<Trait>(i)).shortName);
	traitColour.selected = 0;
	traitColour.tab = DisplayTab;
	traitColour.boxWidth = 78.0f;
	traitColour.boxSpacing = 84.0f;

	overlays.label = "Overlays";
	overlays.help = "Also F keys while running, and F8 lists every binding on screen.";
	overlays.options = {"Stalks F4", "Star names F1", "Probe names F2", "Trails F3"};
	overlays.tab = DisplayTab;
	overlays.multiToggle = true;
	overlays.mask = OverlayStalks; // stalks on, the rest off -- the old defaults
	overlays.boxWidth = 108.0f;
	overlays.boxSpacing = 114.0f;

	firstArrival.label = "Replicate at first star";
	firstArrival.help = "Whether a new probe may copy itself at the very first system it reaches, or must establish itself there first. Roughly doubles the growth exponent.";
	firstArrival.options = {"No", "Yes"};
	firstArrival.selected = 0;

	economy.label = "Resource economy";
	economy.help = "On, a copy has to be mined for and fuel runs out. Off restores free replication, where nothing but the fleet cap ever ends a run.";
	economy.options = {"Off", "On"};
	economy.selected = 1;

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
	lastWindowSize = windowSize; // so a tab switch can re-lay out without being handed it again
	panel = sf::FloatRect((windowSize.x - PANEL_W) * 0.5f, (windowSize.y - PANEL_H) * 0.5f, PANEL_W, PANEL_H);

	// Controls not on the active tab are given empty rectangles rather than merely
	// being skipped when drawing. sf::FloatRect::contains is false for a zero-sized
	// rect, so this makes them genuinely unclickable -- leaving stale geometry behind
	// would let an invisible control on the other tab swallow a click.
	auto hide = [](Segmented &s) {
		s.boxes.assign(s.options.size(), sf::FloatRect());
	};

	float y = panel.top + 158.0f; // leaves room for the tab bar

	preset.boxes.clear();
	if (activeTab == SimulationTab)
	{
		for (size_t i = 0; i < preset.options.size(); ++i)
		{
			preset.boxes.push_back(sf::FloatRect(panel.left + TRACK_X + i * preset.boxSpacing,
												 y - 4.0f, preset.boxWidth, 26.0f));
		}
		y += ROW_H;
	}
	else
	{
		hide(preset);
	}

	for (auto &s : sliders)
	{
		if (s.tab != activeTab)
		{
			s.track = sf::FloatRect();
			continue;
		}
		s.track = sf::FloatRect(panel.left + TRACK_X, y + 4.0f, TRACK_W, 6.0f);
		y += ROW_H;
	}

	auto place = [&](Segmented &s) {
		if (s.tab != activeTab)
		{
			hide(s);
			return;
		}
		s.boxes.clear();
		for (size_t i = 0; i < s.options.size(); ++i)
		{
			s.boxes.push_back(sf::FloatRect(panel.left + TRACK_X + i * s.boxSpacing,
											y - 4.0f, s.boxWidth, 26.0f));
		}
		y += ROW_H;
	};

	place(economy);
	place(evolution);
	place(firstArrival);
	place(traitColour);
	place(trailMode);
	place(trailPaletteControl);
	place(overlays);
	place(spriteStyle);

	// Tab bar, under the heading.
	tabButtons.clear();
	for (int i = 0; i < TabCount; ++i)
	{
		tabButtons.push_back(sf::FloatRect(panel.left + PAD + i * 148.0f, panel.top + 104.0f, 140.0f, 32.0f));
	}

	launchButton = sf::FloatRect(panel.left + PANEL_W - PAD - 210.0f, panel.top + PANEL_H - 74.0f, 210.0f, 44.0f);
}

void SetupUI::setTrailModeChoice(int index)
{
	if (index >= 0 && index < static_cast<int>(trailMode.options.size()))
		trailMode.selected = index;
}

void SetupUI::setOverlay(OverlayBit bit, bool on)
{
	if (on)
		overlays.mask |= static_cast<unsigned int>(bit);
	else
		overlays.mask &= ~static_cast<unsigned int>(bit);
}

void SetupUI::setTraitColourChoice(int index)
{
	if (index >= 0 && index < static_cast<int>(traitColour.options.size()))
		traitColour.selected = index;
}

void SetupUI::setTrailPaletteChoice(int index)
{
	if (index >= 0 && index < static_cast<int>(trailPaletteControl.options.size()))
		trailPaletteControl.selected = index;
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
	// Tabs first: switching one re-lays out every control, so nothing below should
	// act on coordinates from the layout being replaced.
	for (int i = 0; i < static_cast<int>(tabButtons.size()); ++i)
	{
		if (tabButtons[static_cast<size_t>(i)].contains(p))
		{
			if (activeTab != static_cast<Tab>(i))
			{
				activeTab = static_cast<Tab>(i);
				layout(lastWindowSize);
			}
			return true;
		}
	}

	for (size_t i = 0; i < overlays.boxes.size(); ++i)
	{
		if (overlays.boxes[i].contains(p))
		{
			overlays.mask ^= (1u << i); // each chip is independent
			return true;
		}
	}
	for (size_t i = 0; i < evolution.boxes.size(); ++i)
	{
		if (evolution.boxes[i].contains(p))
		{
			evolution.selected = static_cast<int>(i);
			preset.selected = 3; // Custom
			return true;
		}
	}
	for (size_t i = 0; i < traitColour.boxes.size(); ++i)
	{
		if (traitColour.boxes[i].contains(p))
		{
			traitColour.selected = static_cast<int>(i);
			return true;
		}
	}
	for (size_t i = 0; i < trailMode.boxes.size(); ++i)
	{
		if (trailMode.boxes[i].contains(p))
		{
			trailMode.selected = static_cast<int>(i);
			return true;
		}
	}
	for (size_t i = 0; i < trailPaletteControl.boxes.size(); ++i)
	{
		if (trailPaletteControl.boxes[i].contains(p))
		{
			trailPaletteControl.selected = static_cast<int>(i);
			return true;
		}
	}
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
	for (size_t i = 0; i < firstArrival.boxes.size(); ++i)
	{
		if (firstArrival.boxes[i].contains(p))
		{
			firstArrival.selected = static_cast<int>(i);
			preset.selected = 3; // Custom
			return true;
		}
	}
	for (size_t i = 0; i < economy.boxes.size(); ++i)
	{
		if (economy.boxes[i].contains(p))
		{
			economy.selected = static_cast<int>(i);
			preset.selected = 3; // Custom
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
	auto segments = [&](const Segmented &seg, float rowY, bool swatches = false) {
		if (seg.tab != activeTab)
			return;
		text(seg.label, panel.left + PAD, rowY, 16, LABEL);
		text(seg.help, panel.left + PAD, rowY + 19.0f, 12, HELP);
		for (size_t i = 0; i < seg.boxes.size(); ++i)
		{
			const bool on = seg.multiToggle ? ((seg.mask & (1u << i)) != 0)
										    : (static_cast<int>(i) == seg.selected);
			sf::RectangleShape box(sf::Vector2f(seg.boxes[i].width, seg.boxes[i].height));
			box.setPosition(seg.boxes[i].left, seg.boxes[i].top);
			box.setFillColor(on ? SEG_ON : SEG_OFF);
			box.setOutlineThickness(1.0f);
			box.setOutlineColor(on ? HEADING : PANEL_EDGE);
			window.draw(box);

			if (swatches && static_cast<int>(i) < trailPaletteCount())
			{
				// Show the colour itself rather than only its name -- picking "Teal"
				// from a word is guesswork until you have seen it on the map once.
				const TrailPalette &p = trailPalettes()[i];
				sf::RectangleShape swatch(sf::Vector2f(seg.boxes[i].width - 10.0f, 5.0f));
				swatch.setPosition(seg.boxes[i].left + 5.0f, seg.boxes[i].top + 17.0f);
				swatch.setFillColor(sf::Color(p.r, p.g, p.b));
				window.draw(swatch);
				text(seg.options[i], seg.boxes[i].left + 5.0f, seg.boxes[i].top + 1.0f, 11,
					 on ? VALUE : LABEL);
			}
			else
			{
				text(seg.options[i], seg.boxes[i].left + 9.0f, seg.boxes[i].top + 4.0f, 13,
					 on ? VALUE : LABEL);
			}
		}
	};

	text("BEGIN EXPEDITION", panel.left + PAD, panel.top + 30.0f, 28, HEADING);
	text("Mouse sets the controls. Arrow keys still pan the map behind.",
		 panel.left + PAD, panel.top + 68.0f, 13, HELP);

	// --- tab bar ---------------------------------------------------------------
	static const char *TAB_NAMES[] = {"SIMULATION", "DISPLAY"};
	for (size_t i = 0; i < tabButtons.size(); ++i)
	{
		const bool on = (static_cast<int>(i) == static_cast<int>(activeTab));
		sf::RectangleShape tab(sf::Vector2f(tabButtons[i].width, tabButtons[i].height));
		tab.setPosition(tabButtons[i].left, tabButtons[i].top);
		tab.setFillColor(on ? SEG_ON : SEG_OFF);
		tab.setOutlineThickness(1.0f);
		tab.setOutlineColor(on ? HEADING : PANEL_EDGE);
		window.draw(tab);
		text(TAB_NAMES[i], tabButtons[i].left + 14.0f, tabButtons[i].top + 7.0f, 15, on ? VALUE : LABEL);
	}
	text(activeTab == SimulationTab ? "These change the result, and so the score."
									: "These only change how it looks. Nothing here affects the score.",
		 panel.left + PAD + tabButtons.size() * 148.0f + 10.0f, panel.top + 113.0f, 12, HELP);

	if (activeTab == SimulationTab)
		segments(preset, panel.top + 154.0f);

	for (const auto &s : sliders)
	{
		if (s.tab != activeTab)
			continue;
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

	auto rowTop = [&](const Segmented &s) {
		return s.boxes.empty() ? panel.top : s.boxes[0].top + 4.0f;
	};
	segments(economy, rowTop(economy));
	segments(evolution, rowTop(evolution));
	segments(traitColour, rowTop(traitColour));
	segments(firstArrival, rowTop(firstArrival));
	segments(trailMode, rowTop(trailMode));
	segments(trailPaletteControl, rowTop(trailPaletteControl), true);
	segments(overlays, rowTop(overlays));
	segments(spriteStyle, rowTop(spriteStyle));

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
