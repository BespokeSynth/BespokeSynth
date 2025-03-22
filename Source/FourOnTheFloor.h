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

#pragma once

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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 120;
      height = 22;
   }


   bool mTwoOnTheFloor{ false };
   Checkbox* mTwoOnTheFloorCheckbox{ nullptr };
};
