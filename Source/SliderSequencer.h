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
//  SliderSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/25/14.
//
//

#ifndef __Bespoke__SliderSequencer__
#define __Bespoke__SliderSequencer__

#include <iostream>

#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"

class SliderSequencer;

class SliderLine
{
public:
   SliderLine(SliderSequencer* owner, int x, int y, int index);
   void Draw();
   void CreateUIControls();
   
   float mPoint;
   FloatSlider* mSlider;
   float mVelocity;
   FloatSlider* mVelocitySlider;
   int mPitch;
   TextEntry* mNoteSelector;
   double mPlayTime;
   bool mPlaying;
   Checkbox* mPlayingCheckbox;
   int mX;
   int mY;
   SliderSequencer* mOwner;
   int mIndex;
};

class SliderSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IDropdownListener, public IIntSliderListener, public ITextEntryListener
{
public:
   SliderSequencer();
   ~SliderSequencer();
   static IDrawableModule* Create() { return new SliderSequencer(); }
   
   string GetTitleLabel() override { return "slider sequencer"; }
   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool on) override { mEnabled = on; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   float MeasurePos(double time);
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=320; height=165; }
   bool Enabled() const override { return mEnabled; }
   
   float mLastMeasurePos;
   std::vector<SliderLine*> mSliderLines;
   int mDivision;
   IntSlider* mDivisionSlider;
};


#endif /* defined(__Bespoke__SliderSequencer__) */


