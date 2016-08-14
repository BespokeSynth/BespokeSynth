//
//  MidiInstrument.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/23/12.
//
//

#ifndef __modularSynth__MidiInstrument__
#define __modularSynth__MidiInstrument__

#include <iostream>
#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "DropdownList.h"
#include "ModulationChain.h"

class IAudioSource;

class MidiInstrument : public MidiDeviceListener, public IDrawableModule, public INoteSource, public IDropdownListener
{
public:
   MidiInstrument();
   virtual ~MidiInstrument() {}
   static IDrawableModule* Create() { return new MidiInstrument(); }
   
   string GetTitleLabel() override { return mModuleName; }
   void CreateUIControls() override;
   
   void Init() override;
   
   void SetVelocityMult(float mult) { mVelocityMult = mult; }
   void SetUseChannelAsVoice(bool use) { mUseChannelAsVoice = use; }
   void SetNoteOffset(int offset) { mNoteOffset = offset; }
   void SetPitchBendRange(float range) { mPitchBendRange = range; }
   
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPressure(MidiPressure& pressure) override;
   void OnMidiPitchBend(MidiPitchBend& pitchbend) override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
protected:
   void InitController();
   void BuildControllerList();
   
   //IDrawableModule
   virtual void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(int& w, int& h) override { w=190; h=25; }

   int mControllerIndex;
   DropdownList* mControllerList;
   
   string mModuleName;
   MidiDevice mDevice;
   float mVelocityMult;
   bool mUseChannelAsVoice;
   float mCurrentPitchBend;
   int mNoteOffset;
   float mPitchBendRange;
   int mModwheelCC;
   
   Modulations mModulation;
};

#endif /* defined(__modularSynth__MidiInstrument__) */

