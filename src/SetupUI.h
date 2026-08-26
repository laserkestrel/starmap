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
	// Order matters: these index into the slider list.
	enum SliderId
	{
		SearchRadius = 0,
		ReplicationLimit,
		ProbeSpeed,
		FleetCap,
		GalaxySize,
		ViewTilt,
		ViewDepth,
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
	};

	struct Segmented
	{
		std::string label;
		std::string help;
		std::vector<std::string> options;
		int selected = 0;
		std::vector<sf::FloatRect> boxes;
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
	Segmented preset;
	sf::FloatRect panel;
	sf::FloatRect launchButton;
	int draggingSlider = -1;
};
