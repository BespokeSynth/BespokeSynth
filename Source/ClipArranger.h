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
//  ClipArranger.h
//  Bespoke
//
//  Created by Ryan Challinor on 8/26/14.
//
//

#pragma once

#include "IAudioReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "Ramp.h"
#include "Checkbox.h"
#include "NamedMutex.h"
#include "Sample.h"

class ClipArranger : public IDrawableModule, public IFloatSliderListener, public IButtonListener
{
public:
   ClipArranger();
   virtual ~ClipArranger();
   static IDrawableModule* Create() { return new ClipArranger(); }

   void Poll() override;
   void Process(double time, float* left, float* right, int bufferSize);

   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void FilesDropped(std::vector<std::string> files, int x, int y) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   float MouseXToBufferPos(float mouseX);
   int MouseXToSample(float mouseX);
   float SampleToX(int sample);
   bool IsMousePosWithinClip(int x, int y);
   void AddSample(Sample* sample, int x, int y);

   static const int MAX_CLIPS = 50;
   static const int BUFFER_MARGIN_X = 5;
   static const int BUFFER_MARGIN_Y = 5;

   class Clip
   {
   public:
      Clip() {}
      void Process(float* left, float* right, int bufferSize);

      Sample* mSample{ nullptr };
      int mStartSample{ 0 };
      int mEndSample{ 0 };
   };

   enum ClipMoveMode
   {
      kMoveMode_None,
      kMoveMode_Start,
      kMoveMode_End
   } mMoveMode{ ClipMoveMode::kMoveMode_None };

   Clip* GetEmptyClip();

   Clip mClips[MAX_CLIPS];

   float mBufferWidth{ 800 };
   float mBufferHeight{ 80 };
   int mHighlightClip{ -1 };
   bool mMouseDown{ false };
   int mLastMouseX{ -1 };
   int mLastMouseY{ -1 };
   NamedMutex mMutex;
};
