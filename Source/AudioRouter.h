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
//  AudioRouter.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/7/13.
//
//

#ifndef __modularSynth__AudioRouter__
#define __modularSynth__AudioRouter__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "RadioButton.h"

class AudioRouter : public IAudioProcessor, public IDrawableModule, public IRadioButtonListener
{
public:
   AudioRouter();
   virtual ~AudioRouter();
   static IDrawableModule* Create() { return new AudioRouter(); }
   
   string GetTitleLabel() override { return "router"; }
   void CreateUIControls() override;

   void AddReceiver(IAudioReceiver* receiver, const char* name);
   void SetActiveIndex(int index) { mRouteIndex = index; }

   //IAudioSource
   void Process(double time) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* button, int oldVal) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   bool Enabled() const override { return true; }

   int mRouteIndex;
   RadioButton* mRouteSelector;
   vector<IAudioReceiver*> mReceivers;
};


#endif /* defined(__modularSynth__AudioRouter__) */

