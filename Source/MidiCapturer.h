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

    MidiCapturer.h
    Created: 15 Apr 2019 9:32:38pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "Transport.h"
#include "INonstandardController.h"

class MidiDeviceListener;

class MidiCapturerDummyController : public INonstandardController
{
public:
   MidiCapturerDummyController(MidiDeviceListener* listener);
   virtual ~MidiCapturerDummyController() {}

   void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) override {}

   void SendMidi(const juce::MidiMessage& message);

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   MidiDeviceListener* mListener;
};

class MidiCapturer : public NoteEffectBase, public IDrawableModule, public IAudioPoller
{
public:
   MidiCapturer();
   virtual ~MidiCapturer();
   static IDrawableModule* Create() { return new MidiCapturer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void Init() override;

   void AddDummyController(MidiCapturerDummyController* controller);

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendMidi(const juce::MidiMessage& message) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 300;
      height = 150;
   }

   static const int kRingBufferLength = 1000;
   int mRingBufferPos{ 0 };
   juce::MidiMessage mMessages[kRingBufferLength];
   std::list<MidiCapturerDummyController*> mDummyControllers;
   int mPlayhead{ 0 };
};
