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
//  EuclideanSequencer.h
//  Bespoke
//
//  Created by Jack van Klaren on Mar 17 2024.
//  Based on CircleSequencer by Ryan Challinor
//
//

#ifndef __Bespoke__EuclideanSequencer__
#define __Bespoke__EuclideanSequencer__

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
#include "ClickButton.h"

class EuclideanSequencer;

#define EUCLIDEAN_SEQUENCER_MAX_STEPS 32

class EuclideanSequencerRing
{
public:
   EuclideanSequencerRing(EuclideanSequencer* owner, int index);
   void Draw();
   void OnClicked(float x, float y, bool right);
   void Randomize(bool steps, bool onsets, bool rotation);
   void MouseReleased();
   void MouseMoved(float x, float y);
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time);
   void CreateUIControls();
   void OnTransportAdvanced(float amount);
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

private:
   float GetRadius() { return 90 - mIndex * 15; }
   int GetStepIndex(int x, int y, float& radiusOut);
   EuclideanSequencer* mOwner{ nullptr };
   int mIndex{ 0 };
   std::array<float, EUCLIDEAN_SEQUENCER_MAX_STEPS> mSteps{};
   int mLength{ 4 };
   IntSlider* mLengthSlider{ nullptr };
   int mOnset{ 4 };
   IntSlider* mOnsetSlider{ nullptr };
   int mRotation{ 0 };
   IntSlider* mRotationSlider{ nullptr };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };
   int mPitch{ 0 };
   TextEntry* mNoteSelector{ nullptr };
   AdditionalNoteCable* mDestinationCable;


   int mCurrentlyClickedStepIdx{ -1 };
   int mHighlightStepIdx{ -1 };
   float mLastMouseRadius{ -1 };

   std::string GetEuclideanRhythm(int pulses, int steps, int rotation);
};

class EuclideanSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITextEntryListener, public IButtonListener
{
public:
   EuclideanSequencer();
   ~EuclideanSequencer();
   static IDrawableModule* Create() { return new EuclideanSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool on) override { mEnabled = on; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 655;
      height = 200;
   }
   void OnClicked(float x, float y, bool right) override;
   ClickButton* mRandomizeButton{ nullptr };
   ClickButton* mRndLengthButton{ nullptr };
   ClickButton* mRndOnsetsButton{ nullptr };
   ClickButton* mRndRotationButton{ nullptr };
   ClickButton* mRnd0Button{ nullptr };
   ClickButton* mRnd1Button{ nullptr };
   ClickButton* mRnd2Button{ nullptr };
   ClickButton* mRnd3Button{ nullptr };
   void Randomize(bool steps, bool onsets, bool rotation);

   std::vector<EuclideanSequencerRing*> mEuclideanSequencerRings;
};

#endif /* defined(__Bespoke__EuclideanSequencer__) */
