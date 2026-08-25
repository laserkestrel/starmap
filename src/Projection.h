// Projection.h
#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>

// Projection
// ----------
// The single place where world space becomes screen space.
//
// World space is parsecs, in the catalogue's own equatorial Cartesian frame:
// +x towards right ascension 0h on the celestial equator, +y towards 6h, and
// +z towards the north celestial pole. Sol sits at the origin.
//
// The view is a tilted orthographic projection -- imagine looking straight down
// at the z = 0 plane and then leaning back by `tiltDegrees`:
//
//     90 degrees = straight down. z vanishes entirely and stars have no stalk.
//      0 degrees = edge on. You see height but lose all depth in y.
//
// Around 75 degrees reads well: enough tilt for the stalks to be legible,
// not so much that the field smears vertically.
//
// A star's stalk runs from project() to planeFoot(). Both share a screen x, so
// the stalk is always vertical, and its length on screen encodes the star's
// height above or below the plane and nothing else.
class Projection
{
public:
	Projection() { setTiltDegrees(75.0f); }

	void setTiltDegrees(float degrees)
	{
		tiltDegrees = degrees;
		const float radians = degrees * 3.14159265358979f / 180.0f;
		sinTilt = std::sin(radians);
		cosTilt = std::cos(radians);
	}
	void setPixelsPerParsec(float scale) { pixelsPerParsec = scale; }

	// How far above and below the plane the view reaches, in parsecs.
	// An orthographic view is unbounded in depth, so without this every star in
	// the catalogue -- including ones 170 pc off the plane -- projects into frame
	// and the stalks turn into a solid curtain. A real navigation map shows a
	// neighbourhood, not everything.
	void setViewDepthParsecs(float depth) { viewDepthParsecs = depth; }
	float getViewDepthParsecs() const { return viewDepthParsecs; }
	bool withinViewDepth(float z) const { return std::abs(z) <= viewDepthParsecs; }
	void setCentre(const sf::Vector2f &screenCentre) { centre = screenCentre; }

	float getTiltDegrees() const { return tiltDegrees; }
	float getPixelsPerParsec() const { return pixelsPerParsec; }

	// Where a world point lands on screen.
	sf::Vector2f project(float x, float y, float z) const
	{
		return {centre.x + x * pixelsPerParsec,
				centre.y + (y * sinTilt - z * cosTilt) * pixelsPerParsec};
	}

	// Where that point's shadow on the z = 0 plane lands.
	sf::Vector2f planeFoot(float x, float y) const
	{
		return {centre.x + x * pixelsPerParsec,
				centre.y + (y * sinTilt) * pixelsPerParsec};
	}

	// Distance along the view axis. Sort descending on this to draw far stars
	// first and near ones on top.
	float depth(float y, float z) const { return y * cosTilt + z * sinTilt; }

private:
	float pixelsPerParsec = 40.0f;
	float viewDepthParsecs = 15.0f;
	float tiltDegrees = 75.0f;
	float sinTilt = 1.0f;
	float cosTilt = 0.0f;
	sf::Vector2f centre{0.0f, 0.0f};
};
