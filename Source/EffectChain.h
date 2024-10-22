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
//  EffectChain.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Slider.h"
#include "Checkbox.h"
#include "DropdownList.h"

#define MAX_EFFECTS_IN_CHAIN 100
#define MIN_EFFECT_WIDTH 80

class IAudioEffect;

class EffectChain : public IDrawableModule, public IAudioProcessor, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   EffectChain();
   virtual ~EffectChain();
   static IDrawableModule* Create() { return new EffectChain(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void Init() override;
   void Poll() override;
   void AddEffect(std::string type, std::string desiredName, bool onTheFly);
   void SetWideCount(int count) { mNumFXWide = count; }

   //IAudioSource
   void Process(double time) override;

   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;

   bool HasPush2OverrideControls() const override { return true; }
   void GetPush2OverrideControls(std::vector<IUIControl*>& controls) const override;

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   virtual void LoadBasics(const ofxJSONElement& moduleInfo, std::string typeName) override;
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void UpdateOldControlName(std::string& oldName) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

   int GetRowHeight(int row) const;
   int NumRows() const;
   void DeleteEffect(int index);
   void MoveEffect(int index, int direction);
   void UpdateReshuffledDryWetSliders();
   ofVec2f GetEffectPos(int index) const;

   struct EffectControls
   {
      ClickButton* mMoveLeftButton{ nullptr };
      ClickButton* mMoveRightButton{ nullptr };
      ClickButton* mDeleteButton{ nullptr };
      FloatSlider* mDryWetSlider{ nullptr };
      ClickButton* mPush2DisplayEffectButton{ nullptr };
   };

   std::vector<IAudioEffect*> mEffects{};
   ChannelBuffer mDryBuffer;
   std::vector<EffectControls> mEffectControls;
   std::array<float, MAX_EFFECTS_IN_CHAIN> mDryWetLevels{};

   double mSwapTime{ -1 };
   int mSwapFromIdx{ -1 };
   int mSwapToIdx{ -1 };
   ofVec2f mSwapFromPos;
   ofVec2f mSwapToPos;
   float mVolume{ 1 };
   FloatSlider* mVolumeSlider{ nullptr };
   int mNumFXWide{ 3 };
   bool mInitialized{ false };
   bool mShowSpawnList{ true };
   int mWantToDeleteEffectAtIndex{ -1 };
   IAudioEffect* mPush2DisplayEffect{ nullptr };

   std::vector<std::string> mEffectTypesToSpawn;
   int mSpawnIndex{ -1 };
   DropdownList* mEffectSpawnList{ nullptr };
   ClickButton* mSpawnEffectButton{ nullptr };
   ClickButton* mPush2ExitEffectButton{ nullptr };

   ofMutex mEffectMutex;
};
