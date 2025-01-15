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
/*
  ==============================================================================

    NoteQuantizer.h
    Created: 6 Dec 2020 7:36:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "DropdownList.h"
#include "Transport.h"
#include "IPulseReceiver.h"

class NoteQuantizer : public NoteEffectBase, public IDrawableModule, public IDropdownListener, public ITimeListener, public IPulseReceiver
{
public:
   NoteQuantizer();
   virtual ~NoteQuantizer();
   static IDrawableModule* Create() { return new NoteQuantizer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void OnTimeEvent(double time) override;
   void OnPulse(double time, float velocity, int flags) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 80;
      height = 40;
   }
   void OnEvent(double time, float strength);

   struct InputInfo
   {
      NoteMessage note;
      bool held{ false };
      bool hasPlayedYet{ false };
   };

   bool mNoteRepeat{ false };
   Checkbox* mNoteRepeatCheckbox{ nullptr };
   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_16n };
   DropdownList* mQuantizeIntervalSelector{ nullptr };
   std::array<InputInfo, 128> mInputInfos{};
   std::array<bool, 128> mScheduledOffs{};
   std::array<bool, 128> mPreScheduledOffs{};
   bool mHasReceivedPulse{ false };
};
