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

void MidiCapturerDummyController::SendMidi(const MidiMessage& message)
{
   MidiDevice::SendMidiMessage(mListener, "midicapturer", message);
}

MidiCapturer::MidiCapturer()
: mRingBufferPos(0)
, mPlayhead(0)
{
   for (int i=0; i<kRingBufferLength; ++i)
      mMessages[i].setTimeStamp(-1);
   
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

void MidiCapturer::SendMidi(const MidiMessage& message)
{
   //ofLog() << message.getDescription();
   MidiMessage copy = message;
   copy.setTimeStamp(TheTransport->GetMeasureTime(gTime));
   mMessages[mRingBufferPos] = copy;
   mRingBufferPos = (mRingBufferPos + 1) % kRingBufferLength;
}

void MidiCapturer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
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
