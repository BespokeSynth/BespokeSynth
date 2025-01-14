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
//  MidiOutput.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/24/15.
//
//

#pragma once

#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "DropdownList.h"
#include "Transport.h"

class IAudioSource;

class MidiOutputModule : public IDrawableModule, public INoteReceiver, public IDropdownListener, public IAudioPoller
{
public:
   MidiOutputModule();
   virtual ~MidiOutputModule();
   static IDrawableModule* Create() { return new MidiOutputModule(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   void InitController();
   void BuildControllerList();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 190;
      h = 25;
   }

   int mControllerIndex{ -1 };
   DropdownList* mControllerList{ nullptr };

   MidiDevice mDevice{ nullptr };

   int mChannel{ 1 };
   bool mUseVoiceAsChannel{ false };
   float mPitchBendRange{ 2 };
   int mModwheelCC{ 1 }; //or 74 in Multidimensional Polyphonic Expression (MPE) spec

   struct ChannelModulations
   {
      ModulationParameters mModulation;
      float mLastPitchBend{ 0 };
      float mLastModWheel{ 0 };
      float mLastPressure{ 0 };
   };

   std::vector<ChannelModulations> mChannelModulations;
};
