/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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
//  VoiceManager.h
//  Bespoke
//
//  Created by Noxy Nixie on 8/20/2025.
//
//

#pragma once

#include "NoteEffectBase.h"

struct VoiceManagerVoice
{
   VoiceManagerVoice() = default;
   VoiceManagerVoice(int voiceIdx)
   : voiceIdx(voiceIdx)
   {
   }

   int voiceIdx{ 0 };
   NoteMessage lastNote;
   double lastUsedTime{ 0.0 };

   enum VoiceAction
   {
      Unchanged,
      Filter,
      Remap,
   };
   VoiceAction mVoiceAction{ Unchanged };
   DropdownList* voiceActionSelector{ nullptr };
   int mVoiceDestination{ 0 };
   IntSlider* voiceDestinationSlider{ nullptr };
};

class VoiceManager : public NoteEffectBase, public IDrawableModule, public IIntSliderListener, public IDropdownListener
{
public:
   VoiceManager();
   static IDrawableModule* Create() { return new VoiceManager(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void UpdateVoiceUIVisibility();

   void GetModuleDimensions(float& width, float& height) override;

   float mWidth{ 204 };
   float mHeight{ 22 };

   int mLastOutputVoice{ -1 };
   int mVoiceMapping[128]{ 0 };
   std::vector<AdditionalNoteCable*> mDestinationCables;

   enum VoiceSelectionMode
   {
      Rotate,
      Reuse,
      Reset,
      Random,
      Ignore,
   };
   VoiceSelectionMode mVoiceDistributionMode{ Rotate };
   DropdownList* mVoiceDistributionModeSelector{ nullptr };

   int mVoiceLimit{ 16 };
   IntSlider* mVoiceLimitSlider{ nullptr };
   VoiceManagerVoice mVoices[kNumVoices];
};
