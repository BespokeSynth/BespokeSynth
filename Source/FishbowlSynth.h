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
//  FishbowlSynth.h
//  Bespoke
//
//  Created for fishbowl randomizer synth
//
//

#pragma once

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "OpenFrameworksPort.h"

class FishbowlSynth : public IDrawableModule, public INoteSource, public ITimeListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   FishbowlSynth();
   ~FishbowlSynth();
   static IDrawableModule* Create() { return new FishbowlSynth(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void Poll() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   struct Fish
   {
      ofVec2f position;
      ofVec2f velocity;
      ofColor color;
      bool hasTriggered{ false }; // For zone-based triggering
      
      Fish()
      : position(0, 0)
      , velocity(0, 0)
      , color(255, 128, 0)
      {}
   };

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 320;
      height = 380;
   }

   void UpdateFishPhysics();
   void TriggerNoteFromFish(const Fish& fish, double time);
   int FishPositionToPitch(const Fish& fish);
   int FishPositionToVelocity(const Fish& fish);
   float FishPositionToPan(const Fish& fish);
   float FishPositionToPressure(const Fish& fish);
   float FishVelocityToModWheel(const Fish& fish);

   // Fish simulation
   static const int kMaxFish = 10;
   std::vector<Fish> mFish;
   float mBowlRadius{ 140 };
   ofVec2f mBowlCenter{ 160, 190 };

   // Controls
   int mNumFish{ 3 };
   IntSlider* mNumFishSlider{ nullptr };
   
   float mFishSpeed{ 1.0f };
   FloatSlider* mFishSpeedSlider{ nullptr };
   
   NoteInterval mInterval{ NoteInterval::kInterval_8n };
   DropdownList* mIntervalSelector{ nullptr };
   
   int mMinPitch{ 48 };
   IntSlider* mMinPitchSlider{ nullptr };
   
   int mMaxPitch{ 72 };
   IntSlider* mMaxPitchSlider{ nullptr };
   
   float mProbability{ 0.8f };
   FloatSlider* mProbabilitySlider{ nullptr };

   enum TriggerMode
   {
      kTriggerMode_Interval,
      kTriggerMode_ZoneCross,
      kTriggerMode_Both
   };
   
   int mTriggerMode{ kTriggerMode_Interval };
   DropdownList* mTriggerModeSelector{ nullptr };

   // Modulation
   Modulations mModulation{ true }; // Global effect
   bool mModulatePan{ false };
   Checkbox* mModulatePanCheckbox{ nullptr };
   bool mModulatePressure{ false };
   Checkbox* mModulatePressureCheckbox{ nullptr };
   bool mModulateModWheel{ false };
   Checkbox* mModulateModWheelCheckbox{ nullptr };
};

