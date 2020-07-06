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
   
   void SendMidi(const MidiMessage& message);

private:
   MidiDeviceListener* mListener;
};

class MidiCapturer : public NoteEffectBase, public IDrawableModule, public IAudioPoller
{
public:
   MidiCapturer();
   virtual ~MidiCapturer();
   static IDrawableModule* Create() { return new MidiCapturer(); }
   
   string GetTitleLabel() override { return "midi capturer"; }
   
   void AddDummyController(MidiCapturerDummyController* controller);
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendMidi(const MidiMessage& message) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 300; height = 150; }
   bool Enabled() const override { return true; }
   
   static const int kRingBufferLength = 1000;
   int mRingBufferPos;
   MidiMessage mMessages[kRingBufferLength];
   list<MidiCapturerDummyController*> mDummyControllers;
   int mPlayhead;
};
