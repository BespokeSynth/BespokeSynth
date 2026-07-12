/**
    bespoke synth - shared visualizer palettes

    A single set of 24 cosine palettes (Inigo Quilez form: color = a + b*cos(2pi*(c*t + d)))
    shared by every visualizer module (BlobViz, CubeViz, PixelCloud, TextCloud, CheckerBox) so they
    all offer the same named palette list. Self-contained: no dependency on SynthGlobals/ofClamp.
**/

#pragma once

#include <cmath>

struct VizPal
{
   float a[3], b[3], c[3], d[3];
};

// keep this list in sync with kVizPaletteNames below
static const VizPal kVizPalettes[] = {
   { { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.33f, 0.67f } }, //classic
   { { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.30f, 0.20f, 0.20f } }, //neon
   { { 0.70f, 0.60f, 0.70f }, { 0.25f, 0.25f, 0.25f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.15f, 0.30f } }, //pastel
   { { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.00f, 0.00f } }, //mono
   { { 0.60f, 0.40f, 0.30f }, { 0.40f, 0.30f, 0.30f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.10f, 0.20f } }, //sunset
   { { 0.20f, 0.40f, 0.55f }, { 0.20f, 0.30f, 0.40f }, { 1.0f, 1.0f, 1.0f }, { 0.60f, 0.55f, 0.45f } }, //ocean
   { { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 2.0f, 1.0f, 0.0f }, { 0.50f, 0.20f, 0.25f } }, //acid
   { { 0.60f, 0.50f, 0.60f }, { 0.40f, 0.30f, 0.40f }, { 1.0f, 1.0f, 1.0f }, { 0.90f, 0.60f, 0.30f } }, //vaporwave
   { { 0.40f, 0.60f, 0.30f }, { 0.40f, 0.40f, 0.30f }, { 1.0f, 1.0f, 0.5f }, { 0.60f, 0.50f, 0.30f } }, //toxic
   { { 0.50f, 0.18f, 0.18f }, { 0.50f, 0.18f, 0.18f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.05f, 0.10f } }, //bloodmoon
   { { 0.50f, 0.45f, 0.60f }, { 0.50f, 0.40f, 0.50f }, { 1.0f, 1.0f, 1.0f }, { 0.70f, 0.90f, 0.30f } }, //cyberpunk
   { { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 3.0f, 2.0f, 1.0f }, { 0.00f, 0.30f, 0.60f } }, //glitch
   { { 0.18f, 0.20f, 0.35f }, { 0.18f, 0.20f, 0.30f }, { 1.0f, 1.0f, 1.0f }, { 0.60f, 0.65f, 0.75f } }, //midnight
   { { 0.30f, 0.50f, 0.42f }, { 0.30f, 0.40f, 0.40f }, { 1.0f, 1.0f, 1.0f }, { 0.40f, 0.55f, 0.70f } }, //aurora
   { { 0.25f, 0.40f, 0.20f }, { 0.20f, 0.30f, 0.15f }, { 1.0f, 1.0f, 1.0f }, { 0.30f, 0.40f, 0.20f } }, //forest
   { { 0.70f, 0.50f, 0.60f }, { 0.30f, 0.40f, 0.30f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.25f, 0.50f } }, //candy
   { { 0.50f, 0.30f, 0.20f }, { 0.50f, 0.30f, 0.20f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.10f, 0.20f } }, //ember
   { { 0.60f, 0.70f, 0.80f }, { 0.30f, 0.30f, 0.20f }, { 1.0f, 1.0f, 1.0f }, { 0.50f, 0.55f, 0.60f } }, //arctic
   { { 0.40f, 0.28f, 0.50f }, { 0.30f, 0.20f, 0.40f }, { 1.0f, 1.0f, 1.0f }, { 0.70f, 0.90f, 0.60f } }, //grape
   { { 0.50f, 0.44f, 0.34f }, { 0.25f, 0.22f, 0.18f }, { 1.0f, 1.0f, 1.0f }, { 0.10f, 0.12f, 0.15f } }, //sepia
   { { 0.00f, 0.50f, 0.10f }, { 0.00f, 0.50f, 0.10f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.00f, 0.00f } }, //matrix
   { { 0.40f, 0.30f, 0.45f }, { 0.35f, 0.30f, 0.40f }, { 1.0f, 1.0f, 1.0f }, { 0.85f, 0.60f, 0.90f } }, //dracula
   { { 0.52f, 0.50f, 0.44f }, { 0.30f, 0.30f, 0.25f }, { 1.0f, 1.0f, 1.0f }, { 0.55f, 0.50f, 0.35f } }, //solarized
   { { 0.78f, 0.78f, 0.78f }, { 0.20f, 0.20f, 0.20f }, { 1.0f, 1.0f, 1.0f }, { 0.00f, 0.00f, 0.00f } }, //mono light
};

static const char* kVizPaletteNames[] = {
   "classic", "neon", "pastel", "mono", "sunset", "ocean", "acid", "vaporwave", "toxic",
   "bloodmoon", "cyberpunk", "glitch", "midnight", "aurora", "forest", "candy", "ember", "arctic",
   "grape", "sepia", "matrix", "dracula", "solarized", "mono light"
};

static const int kNumVizPalettes = (int)(sizeof(kVizPalettes) / sizeof(kVizPalettes[0]));

inline float VizClamp01(float v)
{
   return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// exposure/brightness multiplier applied to a 0..1 colour (>1 brightens, <1 darkens)
inline void VizExpose(float exposure, float& r, float& g, float& b)
{
   r = VizClamp01(r * exposure);
   g = VizClamp01(g * exposure);
   b = VizClamp01(b * exposure);
}

// t and hueShift in 0..1; writes 0..1 colour
inline void VizPaletteColor(int idx, float t, float hueShift, float& rOut, float& gOut, float& bOut)
{
   if (idx < 0)
      idx = 0;
   if (idx >= kNumVizPalettes)
      idx = kNumVizPalettes - 1;
   const VizPal& p = kVizPalettes[idx];
   const float TWO_PI_F = 6.28318530718f;
   rOut = VizClamp01(p.a[0] + p.b[0] * cosf(TWO_PI_F * (p.c[0] * t + p.d[0] + hueShift)));
   gOut = VizClamp01(p.a[1] + p.b[1] * cosf(TWO_PI_F * (p.c[1] * t + p.d[1] + hueShift)));
   bOut = VizClamp01(p.a[2] + p.b[2] * cosf(TWO_PI_F * (p.c[2] * t + p.d[2] + hueShift)));
}
