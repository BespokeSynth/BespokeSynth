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
//  CircleSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/3/15.
//
//

#ifndef __Bespoke__CircleSequencer__
#define __Bespoke__CircleSequencer__

#include <iostream>
#include "Transport.h"
#include "UIGrid.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"

class CircleSequencer;

#define CIRCLE_SEQUENCER_MAX_STEPS 10

class CircleSequencerRing
{
public:
   CircleSequencerRing(CircleSequencer* owner, int index);
   void Draw();
   void OnClicked(float x, float y, bool right);
   void MouseReleased();
   void MouseMoved(float x, float y);
   void CreateUIControls();
   void OnTransportAdvanced(float amount);
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

private:
   float GetRadius() { return 90 - mIndex * 15; }
   int GetStepIndex(int x, int y, float& radiusOut);
   int mLength{ 4 };
   DropdownList* mLengthSelector{ nullptr };
   int mPitch;
   TextEntry* mNoteSelector{ nullptr };
   CircleSequencer* mOwner{ nullptr };
   int mIndex;
   std::array<float, CIRCLE_SEQUENCER_MAX_STEPS> mSteps;
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };
   int mCurrentlyClickedStepIdx{ -1 };
   int mHighlightStepIdx{ -1 };
   float mLastMouseRadius{ -1 };
};

class CircleSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IDropdownListener, public ITextEntryListener
{
public:
   CircleSequencer();
   ~CircleSequencer();
   static IDrawableModule* Create() { return new CircleSequencer(); }


   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool on) override { mEnabled = on; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override {}

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 400;
      height = 200;
   }
   void OnClicked(float x, float y, bool right) override;
   bool Enabled() const override { return mEnabled; }

   std::vector<CircleSequencerRing*> mCircleSequencerRings;
};

#endif /* defined(__Bespoke__CircleSequencer__) */
