/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    AbletonMoveLCD.h
    Created: 23 April 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "SynthGlobals.h"

#define LCDFONT_STYLE_REGULAR 0
#define LCDFONT_STYLE_BOLD 1
#define LCDFONT_STYLE_ITALIC 2
#define LCDFONT_STYLE_UNDERLINE 16 /* under line glyph */
#define LCDFONT_STYLE_STHROUGH 32 /* strike through glyph */
#define LCDFONT_STYLE_NOAA 64 /* no anti-aliasing */
#define LCDFONT_STYLE_NOKERN 128 /* no kerning */
#define LCDFONT_STYLE_NODEFGLYPH 256 /* don't draw default glyph */
#define LCDFONT_STYLE_NOCACHE 512 /* don't cache rasterized glyph */
#define LCDFONT_STYLE_NOHINTING 1024 /* no auto hinting grid (not used as of now) */
#define LCDFONT_STYLE_RTL 2048 /* render right-to-left */
#define LCDFONT_STYLE_ABS_SIZE 4096 /* scale absoulte height */
#define LCDFONT_STYLE_NOSMOOTH 8192 /* no edge-smoothing for bitmaps */

class AbletonMoveLCD
{
public:
   AbletonMoveLCD();
   virtual ~AbletonMoveLCD();

   void Init();
   void Clear();
   void DrawLCDText(const char* text, int x, int y, int style = LCDFONT_STYLE_REGULAR, int fontSize = 12);
   void DrawRect(int x, int y, int width, int height, bool filled);
   void DrawArrow(int pointX, int pointY, int arrowSize, bool left, bool fill);
   void ClearRect(int x, int y, int width, int height);
   void DrawPixel(int x, int y);
   void TogglePixel(int x, int y);

   uint8_t* GetPixels() const { return mPixels; }
   uint8_t GetPixel(int x, int y) const;
   int GetNumDisplayPixels() const;

   static constexpr int kMoveDisplayWidth = 128;
   static constexpr int kMoveDisplayHeight = 64;
   static constexpr int kTextLineSpacing = 8;

private:
   uint8_t* mPixels{ nullptr };
};
