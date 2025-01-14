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

    MidiCapturer.cpp
    Created: 15 Apr 2019 9:32:38pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "MidiCapturer.h"
#include "SynthGlobals.h"
#include "MidiDevice.h"

MidiCapturerDummyController::MidiCapturerDummyController(MidiDeviceListener* listener)
{
   mListener = listener;
   dynamic_cast<MidiCapturer*>(TheSynth->FindModule("midicapturer"))->AddDummyController(this);
}

void MidiCapturerDummyController::SendMidi(const juce::MidiMessage& message)
{
   MidiDevice::SendMidiMessage(mListener, "midicapturer", message);
}

namespace
{
   const int kSaveStateRev = 1;
}

void MidiCapturerDummyController::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
}

void MidiCapturerDummyController::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}

MidiCapturer::MidiCapturer()
{
   for (int i = 0; i < kRingBufferLength; ++i)
      mMessages[i].setTimeStamp(-1);
}

void MidiCapturer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

MidiCapturer::~MidiCapturer()
{
   TheTransport->RemoveAudioPoller(this);
}

void MidiCapturer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void MidiCapturer::OnTransportAdvanced(float amount)
{
   while (mMessages[mPlayhead].getTimeStamp() > 0 &&
          mMessages[mPlayhead].getTimeStamp() < TheTransport->GetMeasureTime(gTime) - 1)
   {
      for (auto c : mDummyControllers)
         c->SendMidi(mMessages[mPlayhead]);
      mMessages[mPlayhead].setTimeStamp(-1); //TODO(Ryan) remove this
      mPlayhead = (mPlayhead + 1) % kRingBufferLength;
   }
}

void MidiCapturer::AddDummyController(MidiCapturerDummyController* controller)
{
   mDummyControllers.push_back(controller);
}

void MidiCapturer::SendMidi(const juce::MidiMessage& message)
{
   //ofLog() << message.getDescription();
   auto copy = message;
   copy.setTimeStamp(TheTransport->GetMeasureTime(gTime));
   mMessages[mRingBufferPos] = copy;
   mRingBufferPos = (mRingBufferPos + 1) % kRingBufferLength;
}

void MidiCapturer::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);
}

void MidiCapturer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void MidiCapturer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
