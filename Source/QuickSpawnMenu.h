/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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

    QuickSpawnMenu.h
    Created: 22 Oct 2017 7:49:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"

class QuickSpawnMenu : public IDrawableModule
{
public:
   QuickSpawnMenu();
   virtual ~QuickSpawnMenu();
   
   void Init() override;
   void DrawModule() override;
   void SetDimensions(int w, int h) { mWidth = w; mHeight = h; }
   bool HasTitleBar() const override { return false; }
   string GetTitleLabel() override { return ""; }
   bool IsSaveable() override { return false; }
   string GetHoveredModuleTypeName();
   
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   void MouseReleased() override;
   
   bool IsSingleton() const override { return true; }
   
private:
   string GetModuleTypeNameAt(int x, int y);
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   float mWidth;
   float mHeight;
   std::vector<string> mElements;
   char mCurrentMenuChar;
   int mLastHoverX;
   int mLastHoverY;
};

extern QuickSpawnMenu* TheQuickSpawnMenu;
