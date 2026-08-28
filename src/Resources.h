// Resources.h
#ifndef RESOURCES_H
#define RESOURCES_H

#include <cmath>
#include <cstdint>
#include <algorithm>

// Three things a probe needs before it can build a copy of itself.
//
//   metals     -- structure. Common.
//   volatiles  -- ices and organics. Also the propellant, so this is fuel as well
//                 as a build cost, which is what makes a long journey genuinely risky.
//   fissiles   -- heavy elements for the power source. Rare, and clumped, so a
//                 lineage's fate largely turns on whether it finds a lode.
//
// Twelve bytes. See the note in Star.h on why that is affordable.
struct Resources
{
	float metals = 0.0f;
	float volatiles = 0.0f;
	float fissiles = 0.0f;

	Resources() = default;
	Resources(float m, float v, float f) : metals(m), volatiles(v), fissiles(f) {}

	Resources &operator+=(const Resources &o)
	{
		metals += o.metals;
		volatiles += o.volatiles;
		fissiles += o.fissiles;
		return *this;
	}

	Resources &operator-=(const Resources &o)
	{
		metals -= o.metals;
		volatiles -= o.volatiles;
		fissiles -= o.fissiles;
		return *this;
	}

	Resources operator*(float s) const { return Resources(metals * s, volatiles * s, fissiles * s); }

	// Does this hold cover that cost?
	bool covers(const Resources &cost) const
	{
		return metals >= cost.metals && volatiles >= cost.volatiles && fissiles >= cost.fissiles;
	}

	float total() const { return metals + volatiles + fissiles; }

	bool empty() const { return total() <= 0.0001f; }

	// How close this hold is to affording a cost, 0..1, limited by the scarcest
	// component. Useful for asking "what is actually holding this probe back".
	float fractionOf(const Resources &cost) const
	{
		float worst = 1.0f;
		if (cost.metals > 0.0f) worst = std::min(worst, metals / cost.metals);
		if (cost.volatiles > 0.0f) worst = std::min(worst, volatiles / cost.volatiles);
		if (cost.fissiles > 0.0f) worst = std::min(worst, fissiles / cost.fissiles);
		return std::max(0.0f, worst);
	}
};

// ---------------------------------------------------------------------------
// Where a system's richness comes from.
//
// Be clear about what is real and what is invented. The catalogue carries
// position and colour; it does not carry composition, so the abundances below are
// a made-up model, not astrophysics. What matters for the simulation is not that
// they are true but that they are SPATIALLY STRUCTURED: drawing an independent
// random number per star would give uniform noise, every region would average the
// same, and every lineage would meet the same galaxy. Smooth 3D noise instead
// produces lodes and deserts several parsecs across, so which way a lineage
// happens to expand decides whether it thrives -- which is the emergent behaviour
// the whole thing is for.
//
// It is deterministic in world position, so the same seed gives the same galaxy on
// every run. That is what makes two runs with different parameters comparable, and
// what stops the high score table comparing luck.
// ---------------------------------------------------------------------------
namespace ResourceField
{
	// Cheap integer hash -- deterministic, no <random> state, safe to call from
	// several threads because it keeps none.
	inline float hashToUnit(int x, int y, int z, uint32_t seed)
	{
		uint32_t h = seed;
		h ^= static_cast<uint32_t>(x) * 0x8DA6B343u;
		h ^= static_cast<uint32_t>(y) * 0xD8163841u;
		h ^= static_cast<uint32_t>(z) * 0xCB1AB31Fu;
		h ^= h >> 15;
		h *= 0x2C1B3C6Du;
		h ^= h >> 12;
		h *= 0x297A2D39u;
		h ^= h >> 15;
		return static_cast<float>(h & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
	}

	// Trilinear value noise: smooth, so nearby stars have similar richness.
	inline float valueNoise(float x, float y, float z, uint32_t seed)
	{
		const float fx = std::floor(x), fy = std::floor(y), fz = std::floor(z);
		const int ix = static_cast<int>(fx), iy = static_cast<int>(fy), iz = static_cast<int>(fz);
		float tx = x - fx, ty = y - fy, tz = z - fz;

		// Smoothstep the interpolation so the field has no directional creases.
		tx = tx * tx * (3.0f - 2.0f * tx);
		ty = ty * ty * (3.0f - 2.0f * ty);
		tz = tz * tz * (3.0f - 2.0f * tz);

		auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

		const float c000 = hashToUnit(ix, iy, iz, seed);
		const float c100 = hashToUnit(ix + 1, iy, iz, seed);
		const float c010 = hashToUnit(ix, iy + 1, iz, seed);
		const float c110 = hashToUnit(ix + 1, iy + 1, iz, seed);
		const float c001 = hashToUnit(ix, iy, iz + 1, seed);
		const float c101 = hashToUnit(ix + 1, iy, iz + 1, seed);
		const float c011 = hashToUnit(ix, iy + 1, iz + 1, seed);
		const float c111 = hashToUnit(ix + 1, iy + 1, iz + 1, seed);

		const float x00 = lerp(c000, c100, tx);
		const float x10 = lerp(c010, c110, tx);
		const float x01 = lerp(c001, c101, tx);
		const float x11 = lerp(c011, c111, tx);

		return lerp(lerp(x00, x10, ty), lerp(x01, x11, ty), tz);
	}

	// Two octaves: broad regions with some finer variation inside them.
	// featureParsecs sets how big a "region" is.
	inline float field(float wx, float wy, float wz, float featureParsecs, uint32_t seed)
	{
		const float s = 1.0f / std::max(0.5f, featureParsecs);
		const float coarse = valueNoise(wx * s, wy * s, wz * s, seed);
		const float fine = valueNoise(wx * s * 3.1f, wy * s * 3.1f, wz * s * 3.1f, seed ^ 0x9E3779B9u);
		return std::min(1.0f, std::max(0.0f, coarse * 0.7f + fine * 0.3f));
	}

	// A system's starting stocks.
	//
	// colourIndex is the star's measured B-V, the one genuinely observational input:
	// volatiles are biased towards cooler, redder stars, on the loose reasoning that a
	// cool star's ice line sits close in, so its condensed volatiles are where a probe
	// can reach them. Metals follow a broad field. Fissiles use a tighter field raised
	// to a power, which leaves most systems with almost none and a few with plenty --
	// deliberately the bottleneck.
	inline Resources forSystem(float wx, float wy, float wz, float colourIndex,
							   float scale, float featureParsecs, uint32_t seed)
	{
		const float metalField = field(wx, wy, wz, featureParsecs, seed);
		const float volatileField = field(wx, wy, wz, featureParsecs * 0.75f, seed ^ 0x51ED2701u);
		const float fissileField = field(wx, wy, wz, featureParsecs * 0.5f, seed ^ 0xA13F77C5u);

		// B-V roughly -0.4 (hot blue) .. 2.0 (cool red). Map to 0..1 and let it
		// shift the volatile yield by +/- 40%.
		const float bv = std::max(-0.4f, std::min(2.0f, colourIndex));
		const float coolness = (bv + 0.4f) / 2.4f;
		const float volatileBias = 0.8f + 0.4f * coolness;

		const Resources r(
			scale * (0.25f + 0.75f * metalField),
			scale * (0.20f + 0.80f * volatileField) * volatileBias,
			scale * std::pow(fissileField, 3.5f) * 1.6f);
		return r;
	}
}

#endif // RESOURCES_H
