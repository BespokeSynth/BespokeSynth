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
//
//  Minimap.h
//  Bespoke
//
//  Created by Ryan Challinor on 09/25/21.
//
//

#pragma once

#include "IDrawableModule.h"
#include "UIGrid.h"


enum class MinimapCorner
{
   TopRight,
   TopLeft,
   BottomRight,
   BottomLeft
};

class Minimap : public IDrawableModule
{
public:
   Minimap();
   ~Minimap();

   void CreateUIControls() override;
   void DrawModule() override;

   bool AlwaysOnTop() override { return true; };
   void GetDimensions(double& width, double& height) override;
   void GetDimensionsMinimap(float& width, float& height);

private:
   bool IsSingleton() const override { return true; };
   bool HasTitleBar() const override { return false; };
   bool IsSaveable() override { return false; }
   void ComputeBoundingBox(ofRectangle_f& rect);
   ofRectangle_f CoordsToMinimap(ofRectangle_f& boundingBox, ofRectangle_f& source);
   void DrawModulesOnMinimap(ofRectangle_f& boundingBox);
   void DrawModuleOnMinimap(ofRectangle_f& boundingBox, IDrawableModule* module);
   void RectUnion(ofRectangle_f& target, ofRectangle_f& unionRect);
   void OnClicked(double x, double y, bool right) override;
   void MouseReleased() override;
   bool MouseMoved(double x, double y) override;
   ofVec2f CoordsToViewport(ofRectangle_f& boundingBox, float x, float y);
   void ForcePosition();

   bool mClick{ false };
   UIGrid* mGrid{ nullptr };
   GridCell mHoveredBookmarkPos{ -1, -1 };
};
