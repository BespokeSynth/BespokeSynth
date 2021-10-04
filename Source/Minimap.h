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

#ifndef __Bespoke__Minimap__
#define __Bespoke__Minimap__

#include "IDrawableModule.h"

class Minimap : public IDrawableModule
{

  public:
    Minimap();
    ~Minimap();
    void DrawModule() override;
    bool AlwaysOnTop() override;
    string GetTitleLabel() override;
    void GetDimensions(float & width, float & height) override;
  private:
    bool IsSingleton() const override;
    bool HasTitleBar() const override;
    void ComputeBoundingBox(ofRectangle & rect);
    ofRectangle CoordsToMinimap(ofRectangle & boundingBox, ofRectangle & source);
    void DrawModulesOnMinimap();
    void DrawModulesOnMinimap(ofRectangle & boundingBox);
    void RectUnion(ofRectangle & target, ofRectangle & unionRect);
    void OnClicked(int x, int y, bool right) override;
    ofVec2f CoordsToViewport(ofRectangle & boundingBox, float x, float y);
    ofVec2f GetPosition(bool local) const;
    void ForcePosition();
    
};

#endif /* defined(__Bespoke__Minimap__) */
