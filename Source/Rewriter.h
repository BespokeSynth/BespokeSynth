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
//  Rewriter.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "MidiDevice.h"
#include "Checkbox.h"

class Looper;

class Rewriter : public IAudioProcessor, public IDrawableModule, public IButtonListener
{
public:
   Rewriter();
   virtual ~Rewriter();
   static IDrawableModule* Create() { return new Rewriter(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Go(double time);

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IAudioSource
   void Process(double time) override;

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   float mWidth{ 200 };
   float mHeight{ 20 };

   double mStartRecordTime{ -1 };

   ClickButton* mRewriteButton{ nullptr };
   ClickButton* mStartRecordTimeButton{ nullptr };

   RollingBuffer mRecordBuffer;
   Looper* mConnectedLooper{ nullptr };

   PatchCableSource* mLooperCable{ nullptr };
};
