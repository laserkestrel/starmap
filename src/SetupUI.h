// SetupUI.h
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// The "Begin Expedition" screen.
//
// Every control is driven by the MOUSE. That is deliberate: the arrow keys pan the
// camera, and the old setup screen used them to change values too, so the two fought
// each other. Splitting them by device means neither has to compromise.
//
// SFML has no widget toolkit, so these are drawn by hand -- which at least means each
// parameter gets the control that actually suits it rather than one size fitting all.
class SetupUI
{
public:
	// Which tab a control belongs to. Splitting them matters for more than tidiness:
	// everything on Simulation changes the result and therefore the score, everything
	// on Display only changes how it looks. Mixing the two made it easy to nudge a
	// parameter while meaning to adjust the view.
	enum Tab
	{
		SimulationTab = 0,
		DisplayTab,
		TabCount
	};

	// Order matters: these index into the slider list.
	enum SliderId
	{
		SearchRadius = 0,
		ReplicationLimit,
		ProbeSpeed,
		FleetCap,
		GalaxySize,
		SystemRichness,
		ProbeBuildCost,
		FuelBurn,
		MutationStrength,
		ViewTilt,
		ViewDepth,
		TrailFade,
		SliderCount
	};

	struct Slider
	{
		std::string label;
		std::string help;   // what it does, and which way it pushes the result
		std::string suffix;
		float minValue = 0.0f;
		float maxValue = 1.0f;
		float value = 0.5f;
		int decimals = 0;
		// When set, the slider snaps to these values only. Used where a linear
		// range would be useless -- a fleet cap wants 10k, 100k, 1m, not 431,772.
		std::vector<float> allowed;
		sf::FloatRect track;
		Tab tab = SimulationTab;
	};

	struct Segmented
	{
		std::string label;
		std::string help;
		std::vector<std::string> options;
		int selected = 0;
		std::vector<sf::FloatRect> boxes;
		Tab tab = SimulationTab;
		// When set, every box is an independent on/off chip and `mask` holds their
		// states, rather than `selected` naming one winner. Lets four related toggles
		// share a single row instead of taking four.
		bool multiToggle = false;
		unsigned int mask = 0;
		// The palette control has eight options and will not fit at the default
		// width, so the box size is per control rather than a constant.
		float boxWidth = 86.0f;
		float boxSpacing = 92.0f;
	};

	SetupUI();

	void layout(const sf::Vector2u &windowSize);
	// Returns true if a value changed.
	bool onMousePressed(const sf::Vector2f &p);
	bool onMouseMoved(const sf::Vector2f &p);
	void onMouseReleased();
	bool launchClicked(const sf::Vector2f &p) const;

	void draw(sf::RenderWindow &window, const sf::Font &font, const std::string &reachSummary) const;

	float value(SliderId id) const { return sliders[id].value; }
	int intValue(SliderId id) const;
	int spriteChoice() const { return spriteStyle.selected; }
	bool replicateOnFirstArrival() const { return firstArrival.selected == 1; }
	void setReplicateOnFirstArrival(bool on) { firstArrival.selected = on ? 1 : 0; }
	bool evolutionEnabled() const { return evolution.selected == 1; }
	void setEvolutionEnabled(bool on) { evolution.selected = on ? 1 : 0; }
	int traitColourChoice() const { return traitColour.selected; }
	void setTraitColourChoice(int index);
	bool resourcesEnabled() const { return economy.selected == 1; }
	void setResourcesEnabled(bool on) { economy.selected = on ? 1 : 0; }
	int trailModeChoice() const { return trailMode.selected; }
	void setTrailModeChoice(int index);
	int trailPaletteChoice() const { return trailPaletteControl.selected; }
	void setTrailPaletteChoice(int index);

	// Overlay toggles, so the F keys do not have to be remembered.
	enum OverlayBit
	{
		OverlayStalks = 1 << 0,
		OverlayStarNames = 1 << 1,
		OverlayProbeNames = 1 << 2,
		OverlayTrails = 1 << 3
	};
	bool overlayOn(OverlayBit bit) const { return (overlays.mask & static_cast<unsigned int>(bit)) != 0; }
	void setOverlay(OverlayBit bit, bool on);
	int presetChoice() const { return preset.selected; }
	void applyPreset(int index);
	// Seeds a control from config so the screen opens showing what the file says
	// rather than a hard-coded default. Snaps to the nearest allowed step where
	// the slider has one.
	void setValue(SliderId id, float v);
	void setSpriteChoice(int index);

private:
	void setFromTrack(Slider &s, float mouseX);

	std::vector<Slider> sliders;
	Segmented spriteStyle;
	Segmented firstArrival;
	Segmented economy;
	Segmented preset;
	Segmented trailMode;
	Segmented trailPaletteControl;
	Segmented overlays;
	Segmented evolution;
	Segmented traitColour;
	sf::FloatRect panel;
	sf::FloatRect launchButton;
	std::vector<sf::FloatRect> tabButtons;
	Tab activeTab = SimulationTab;
	sf::Vector2u lastWindowSize{1280, 720};
	int draggingSlider = -1;
};
