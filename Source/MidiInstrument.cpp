//
//  MidiInstrument.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/23/12.
//
//

#include "MidiInstrument.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "MidiController.h"

MidiInstrument::MidiInstrument()
: mControllerIndex(-1)
, mControllerList(NULL)
, mDevice(this)
, mVelocityMult(1)
, mUseChannelAsVoice(false)
, mNoteOffset(0)
, mCurrentPitchBend(0)
, mPitchBendRange(2)
, mModulation(true)
, mModwheelCC(1)  //or 74 in Multidimensional Polyphonic Expression (MPE) spec
{
   SetIsNoteOrigin(true);
}

void MidiInstrument::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControllerList = new DropdownList(this,"controller",5,5,&mControllerIndex);
}

void MidiInstrument::Init()
{
   IDrawableModule::Init();
   InitController();
}

void MidiInstrument::InitController()
{
   MidiController* controller = TheSynth->FindMidiController(mModuleSaveData.GetString("controller"));
   if (controller)
      mDevice.ConnectInput(controller->GetDeviceIn().c_str());
   
   if (mDevice.IsConnected())
      mModuleName = mDevice.Name();
   
   BuildControllerList();
   
   const std::vector<string>& devices = mDevice.GetPortList();
   for (int i=0; i<devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(),mDevice.Name()) == 0)
         mControllerIndex = i;
   }
}

void MidiInstrument::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mControllerList->Draw();
}

void MidiInstrument::OnMidiNote(MidiNote& note)
{
   if (!mEnabled)
      return;
   
   int voiceIdx = -1;
   
   if (mUseChannelAsVoice)
      voiceIdx = note.mChannel - 1;
   
   PlayNoteOutput(gTime, note.mPitch + mNoteOffset, MIN(127,note.mVelocity*mVelocityMult), voiceIdx, mModulation.GetPitchBend(voiceIdx), mModulation.GetModWheel(voiceIdx), mModulation.GetPressure(voiceIdx));
}

void MidiInstrument::OnMidiControl(MidiControl& control)
{
   if (!mEnabled)
      return;
   
   int voiceIdx = -1;
   
   if (mUseChannelAsVoice)
      voiceIdx = control.mChannel - 1;
   
   if (control.mControl == mModwheelCC)
   {
      mModulation.GetModWheel(voiceIdx)->SetValue(control.mValue / 127.0f);
   }
}

void MidiInstrument::OnMidiPressure(MidiPressure& pressure)
{
   if (!mEnabled)
      return;
   
   int voiceIdx = -1;
   
   if (mUseChannelAsVoice)
      voiceIdx = pressure.mChannel - 1;
   
   mModulation.GetPressure(voiceIdx)->SetValue(pressure.mPressure / 127.0f);
   
   mNoteOutput.SendPressure(pressure.mPitch, pressure.mPressure);
}

void MidiInstrument::OnMidiPitchBend(MidiPitchBend& pitchbend)
{
   if (!mEnabled)
      return;
   
   int voiceIdx = -1;
   
   float amount = (pitchbend.mValue - 8192.0f) / (8192.0f/mPitchBendRange);
   
   if (mUseChannelAsVoice)
      voiceIdx = pitchbend.mChannel - 1;
   else
      mCurrentPitchBend = amount;
   
   mModulation.GetPitchBend(voiceIdx)->SetValue(amount);
}

void MidiInstrument::BuildControllerList()
{
   mControllerList->Clear();
   const std::vector<string>& devices = mDevice.GetPortList();
   for (int i=0; i<devices.size(); ++i)
      mControllerList->AddLabel(devices[i].c_str(), i);
}

void MidiInstrument::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void MidiInstrument::DropdownUpdated(DropdownList* list, int oldVal)
{
   mDevice.ConnectInput(mControllerIndex);
   if (mDevice.IsConnected())
      mModuleName = mDevice.Name();
}

void MidiInstrument::DropdownClicked(DropdownList* list)
{
   BuildControllerList();
}

void MidiInstrument::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("controller",moduleInfo,"",FillDropdown<MidiController*>);
   mModuleSaveData.LoadFloat("velocitymult",moduleInfo,1,0,10,K(isTextField));
   mModuleSaveData.LoadBool("usechannelasvoice",moduleInfo,false);
   mModuleSaveData.LoadInt("noteoffset",moduleInfo,0,-999,999,K(isTextField));
   mModuleSaveData.LoadFloat("pitchbendrange",moduleInfo,2,1,24,K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)",moduleInfo,1,0,127,K(isTextField));

   SetUpFromSaveData();
}

void MidiInstrument::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetVelocityMult(mModuleSaveData.GetFloat("velocitymult"));
   SetUseChannelAsVoice(mModuleSaveData.GetBool("usechannelasvoice"));
   SetNoteOffset(mModuleSaveData.GetInt("noteoffset"));
   SetPitchBendRange(mModuleSaveData.GetFloat("pitchbendrange"));
   
   mModuleName = mModuleSaveData.GetString("controller");
   mModwheelCC = mModuleSaveData.GetInt("modwheelcc(1or74)");
   
   InitController();
}
