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

    AbletonMoveLCD.cpp
    Created: 23 April 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AbletonMoveLCD.h"

#if BESPOKE_WINDOWS
#define _STRING_H_ 1
#define size_t uint32
#endif

#define SSFN_IMPLEMENTATION
#define SSFN_memcmp memcmp
#define SSFN_memset memset
#define SSFN_realloc realloc
#define SSFN_free free
#include "ssfn.h"

namespace
{
   ssfn_t ssfn_ctx;
   ssfn_buf_t ssfn_buf;
   ssfn_font_t* ssfn_font;

   ssfn_font_t* load_file(const char* filename, int* size)
   {
      char* fontdata = NULL;
      FILE* f;

      f = fopen(filename, "rb");
      if (!f)
      {
         fprintf(stderr, "unable to load %s\n", filename);
         exit(3);
      }
      *size = 0;
      fseek(f, 0, SEEK_END);
      *size = (int)ftell(f);
      fseek(f, 0, SEEK_SET);
      if (!*size)
      {
         fprintf(stderr, "unable to load %s\n", filename);
         exit(3);
      }
      fontdata = (char*)malloc(*size);
      if (!fontdata)
      {
         fprintf(stderr, "memory allocation error\n");
         exit(2);
      }
      fread(fontdata, *size, 1, f);
      fclose(f);
      return (ssfn_font_t*)fontdata;
   }

   void set_up_ssfn_font(uint8_t* pixels, int width, int height)
   {
      int ret, size;

      /* initialize the normal renderer */
      memset(&ssfn_ctx, 0, sizeof(ssfn_t));
      ssfn_buf.ptr = pixels;
      ssfn_buf.p = width * 4;
      ssfn_buf.w = width;
      ssfn_buf.h = height;
      ssfn_buf.fg = 0xFFFFFFFF;
      ssfn_buf.bg = 0;

      /* load and select a font */
      ssfn_font = load_file(ofToResourcePath("galmuri7.sfn.gz").c_str(), &size);
      ret = ssfn_load(&ssfn_ctx, ssfn_font);
      if (ret != SSFN_OK)
      {
         fprintf(stderr, "ssfn load error: err=%d %s\n", ret, ssfn_error(ret));
         exit(2);
      }
   }

   void draw_ssfn_text(const char* text, int x, int y, int style = SSFN_STYLE_REGULAR, int fontSize = 12)
   {
      int ret;

      ret = ssfn_select(&ssfn_ctx, SSFN_FAMILY_ANY, NULL, style | SSFN_STYLE_NOAA, fontSize);
      if (ret != SSFN_OK)
      {
         fprintf(stderr, "ssfn select error: err=%d %s\n", ret, ssfn_error(ret));
         exit(2);
      }

      ssfn_buf.x = x;
      ssfn_buf.y = y;

      const char* str = text;
      while ((ret = ssfn_render(&ssfn_ctx, &ssfn_buf, str)) > 0)
         str += ret;
   }
}

AbletonMoveLCD::AbletonMoveLCD()
{
}

AbletonMoveLCD::~AbletonMoveLCD()
{
   ssfn_free(&ssfn_ctx);
   free(ssfn_font);
}

void AbletonMoveLCD::Init()
{
   mPixels = new uint8_t[GetNumDisplayPixels()];

   set_up_ssfn_font(mPixels, kMoveDisplayWidth, kMoveDisplayHeight);
}

void AbletonMoveLCD::Clear()
{
   if (mPixels)
      memset(mPixels, 0, sizeof(uint8_t) * GetNumDisplayPixels());
}

void AbletonMoveLCD::DrawLCDText(const char* text, int x, int y, int style, int fontSize)
{
   draw_ssfn_text(text, x, y, style, fontSize);
}

void AbletonMoveLCD::DrawRect(int x, int y, int width, int height, bool filled)
{
   for (int penX = x; penX < x + width && penX >= 0 && penX < kMoveDisplayWidth; ++penX)
   {
      for (int penY = y; penY < y + height && penY >= 0 && penY < kMoveDisplayHeight; ++penY)
      {
         if (filled || penX == x || penY == y || penX == x + width - 1 || penY == y + height - 1)
            mPixels[(penX + penY * kMoveDisplayWidth) * 4] = 255;
      }
   }
}

void AbletonMoveLCD::DrawArrow(int pointX, int pointY, int arrowSize, bool left, bool fill)
{
   for (int arrowX = 0; arrowX < arrowSize; ++arrowX)
   {
      for (int arrowY = 0; arrowY < arrowSize; ++arrowY)
      {
         if (arrowX == arrowY || (fill && arrowY < arrowX) || arrowX == arrowSize - 1)
         {
            DrawPixel(pointX + arrowX * (left ? 1 : -1), pointY - arrowY);
            DrawPixel(pointX + arrowX * (left ? 1 : -1), pointY + arrowY);
         }
      }
   }
}

void AbletonMoveLCD::ClearRect(int x, int y, int width, int height)
{
   for (int penX = x; penX < x + width && penX >= 0 && penX < kMoveDisplayWidth; ++penX)
   {
      for (int penY = y; penY < y + height && penY >= 0 && penY < kMoveDisplayHeight; ++penY)
      {
         mPixels[(penX + penY * kMoveDisplayWidth) * 4] = 0;
      }
   }
}

void AbletonMoveLCD::DrawPixel(int x, int y)
{
   if (x >= 0 && y >= 0 && x < kMoveDisplayWidth && y < kMoveDisplayHeight)
      mPixels[(x + y * kMoveDisplayWidth) * 4] = 255;
}

void AbletonMoveLCD::TogglePixel(int x, int y)
{
   if (x >= 0 && y >= 0 && x < kMoveDisplayWidth && y < kMoveDisplayHeight)
   {
      if (mPixels[(x + y * kMoveDisplayWidth) * 4] > 0)
         mPixels[(x + y * kMoveDisplayWidth) * 4] = 0;
      else
         mPixels[(x + y * kMoveDisplayWidth) * 4] = 255;
   }
}

uint8_t AbletonMoveLCD::GetPixel(int x, int y) const
{
   if (x >= 0 && y >= 0 && x < kMoveDisplayWidth && y < kMoveDisplayHeight)
      return mPixels[(x + y * kMoveDisplayWidth) * 4];
   return 0;
}

int AbletonMoveLCD::GetNumDisplayPixels() const
{
   return kMoveDisplayWidth * kMoveDisplayHeight * 4;
}
