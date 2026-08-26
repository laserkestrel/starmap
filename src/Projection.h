// Projection.h
#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>

// Projection
// ----------
// The single place where world space becomes screen space, and the camera.
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
// `worldCentre` is the point on the plane the camera is looking at, in parsecs,
// so panning is measured in parsecs and behaves identically at any zoom.
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
		tiltDegrees = std::max(1.0f, std::min(90.0f, degrees));
		const float radians = tiltDegrees * 3.14159265358979f / 180.0f;
		sinTilt = std::sin(radians);
		cosTilt = std::cos(radians);
	}
	void setPixelsPerParsec(float scale) { pixelsPerParsec = std::max(0.0001f, scale); }
	void setScreenCentre(const sf::Vector2f &centre) { screenCentre = centre; }
	void setWorldCentre(const sf::Vector2f &centre) { worldCentre = centre; }

	// How far above and below the plane the view reaches, in parsecs.
	// An orthographic view is unbounded in depth, so without this every star in
	// the catalogue -- including ones 170 pc off the plane -- projects into frame
	// and the stalks turn into a solid curtain.
	void setViewDepthParsecs(float depth) { viewDepthParsecs = std::max(0.1f, depth); }

	float getTiltDegrees() const { return tiltDegrees; }
	float getPixelsPerParsec() const { return pixelsPerParsec; }
	float getViewDepthParsecs() const { return viewDepthParsecs; }
	sf::Vector2f getWorldCentre() const { return worldCentre; }
	bool withinViewDepth(float z) const { return std::abs(z) <= viewDepthParsecs; }

	sf::Vector2f project(float x, float y, float z) const
	{
		return {screenCentre.x + (x - worldCentre.x) * pixelsPerParsec,
				screenCentre.y + ((y - worldCentre.y) * sinTilt - z * cosTilt) * pixelsPerParsec};
	}

	sf::Vector2f planeFoot(float x, float y) const
	{
		return {screenCentre.x + (x - worldCentre.x) * pixelsPerParsec,
				screenCentre.y + ((y - worldCentre.y) * sinTilt) * pixelsPerParsec};
	}

	// Inverse of planeFoot: which point on the z = 0 plane a screen pixel is over.
	sf::Vector2f screenToPlane(const sf::Vector2f &screenPoint) const
	{
		return {worldCentre.x + (screenPoint.x - screenCentre.x) / pixelsPerParsec,
				worldCentre.y + (screenPoint.y - screenCentre.y) / (pixelsPerParsec * sinTilt)};
	}

	// Distance along the view axis. Sort descending on this to draw far stars
	// first and near ones on top.
	float depth(float y, float z) const { return y * cosTilt + z * sinTilt; }

	// --- camera controls --------------------------------------------------------

	void panByParsecs(float dx, float dy) { worldCentre.x += dx; worldCentre.y += dy; }

	// Screen-space panning, so a key press moves the view by the same number of
	// pixels no matter how far zoomed in you are.
	void panByPixels(float dxPixels, float dyPixels)
	{
		worldCentre.x += dxPixels / pixelsPerParsec;
		worldCentre.y += dyPixels / (pixelsPerParsec * sinTilt);
	}

	// Zoom keeping whatever is under `anchor` in the same place on screen. Without
	// the anchor correction the view slides out from under the cursor.
	void zoomAbout(const sf::Vector2f &anchor, float factor, float minScale, float maxScale)
	{
		const sf::Vector2f before = screenToPlane(anchor);
		setPixelsPerParsec(std::max(minScale, std::min(maxScale, pixelsPerParsec * factor)));
		const sf::Vector2f after = screenToPlane(anchor);
		worldCentre.x += before.x - after.x;
		worldCentre.y += before.y - after.y;
	}

	// The rectangle of the plane currently on screen, widened to account for the
	// fact that a star's height can push it into view from outside. Used to ask
	// the quadtree for just the stars that might be visible.
	sf::FloatRect visibleWorldBounds(unsigned int screenWidth, unsigned int screenHeight, float marginParsecs = 1.0f) const
	{
		const float halfW = static_cast<float>(screenWidth) / pixelsPerParsec;
		const float zReach = viewDepthParsecs * cosTilt / sinTilt;

		const float left = worldCentre.x - (screenCentre.x / pixelsPerParsec) - marginParsecs;
		const float right = worldCentre.x + ((screenWidth - screenCentre.x) / pixelsPerParsec) + marginParsecs;
		const float top = worldCentre.y - (screenCentre.y / (pixelsPerParsec * sinTilt)) - zReach - marginParsecs;
		const float bottom = worldCentre.y + ((screenHeight - screenCentre.y) / (pixelsPerParsec * sinTilt)) + zReach + marginParsecs;

		(void)halfW;
		return sf::FloatRect(left, top, right - left, bottom - top);
	}

private:
	float pixelsPerParsec = 40.0f;
	float viewDepthParsecs = 8.0f;
	float tiltDegrees = 75.0f;
	float sinTilt = 1.0f;
	float cosTilt = 0.0f;
	sf::Vector2f screenCentre{0.0f, 0.0f};
	sf::Vector2f worldCentre{0.0f, 0.0f};
};
