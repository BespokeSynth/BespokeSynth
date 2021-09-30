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
//  FourOnTheFloor.h
//  modularSynth
//
//  Created by Ryan Challinor on 6/23/13.
//
//

#ifndef __modularSynth__FourOnTheFloor__
#define __modularSynth__FourOnTheFloor__

#include <iostream>
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Checkbox.h"

class FourOnTheFloor : public IDrawableModule, public INoteSource, public ITimeListener
{
public:
   FourOnTheFloor();
   ~FourOnTheFloor();
   static IDrawableModule* Create() { return new FourOnTheFloor(); }
   
   std::string GetTitleLabel() override { return "four on the floor"; }
   void CreateUIControls() override;
   void Init() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=22; }
   bool Enabled() const override { return mEnabled; }
   
   
   bool mTwoOnTheFloor;
   Checkbox* mTwoOnTheFloorCheckbox;
};

#endif /* defined(__modularSynth__FourOnTheFloor__) */

