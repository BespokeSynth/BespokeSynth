//
//  MidiOutput.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/24/15.
//
//

#include "MidiOutput.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "ModulationChain.h"
#include "PolyphonyMgr.h"
#include "MidiController.h"

namespace
{
   const int kGlobalModulationIdx = 16;
}

MidiOutputModule::MidiOutputModule()
: mControllerIndex(-1)
, mControllerList(nullptr)
, mDevice(nullptr)
, mChannel(1)
, mPitchBendRange(2)
, mModwheelCC(1)  //or 74 in Multidimensional Polyphonic Expression (MPE) spec
, mUseVoiceAsChannel(false)
{
   mChannelModulations.resize(kGlobalModulationIdx+1);
   
   TheTransport->AddAudioPoller(this);
}

MidiOutputModule::~MidiOutputModule()
{
   TheTransport->RemoveAudioPoller(this);
}

void MidiOutputModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControllerList = new DropdownList(this,"controller",5,5,&mControllerIndex);
}

void MidiOutputModule::Init()
{
   IDrawableModule::Init();
   
   InitController();
}

void MidiOutputModule::InitController()
{
   MidiController* controller = TheSynth->FindMidiController(mModuleSaveData.GetString("controller"));
   if (controller)
      mDevice.ConnectOutput(controller->GetDeviceOut().c_str(), mModuleSaveData.GetInt("channel"));
   
   BuildControllerList();
   
   const std::vector<string>& devices = mDevice.GetPortList(false);
   for (int i=0; i<devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(),mDevice.Name()) == 0)
         mControllerIndex = i;
   }
}

void MidiOutputModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mControllerList->Draw();
}

void MidiOutputModule::BuildControllerList()
{
   mControllerList->Clear();
   const std::vector<string>& devices = mDevice.GetPortList(false);
   for (int i=0; i<devices.size(); ++i)
      mControllerList->AddLabel(devices[i].c_str(), i);
}

void MidiOutputModule::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int channel = voiceIdx + 1;
   if (voiceIdx == -1)
      channel = 1;
   
   mDevice.SendNote(pitch, velocity, false, mUseVoiceAsChannel ? channel : mChannel);
   
   int modIdx = voiceIdx;
   if (voiceIdx == -1)
      modIdx = kGlobalModulationIdx;
   
   mChannelModulations[modIdx].mModulation = modulation;
}

void MidiOutputModule::SendCC(int control, int value, int voiceIdx /*=-1*/)
{
   int channel = voiceIdx + 1;
   if (voiceIdx == -1)
      channel = 1;
   
   mDevice.SendCC(control, value, mUseVoiceAsChannel ? channel : mChannel);
}

void MidiOutputModule::OnTransportAdvanced(float amount)
{
   for (int i=0; i<mChannelModulations.size(); ++i)
   {
      ChannelModulations& mod = mChannelModulations[i];
      int channel = i + 1;
      if (i == kGlobalModulationIdx)
         channel = 1;
      float bend = mod.mModulation.pitchBend ? mod.mModulation.pitchBend->GetValue(0) : 0;
      if (bend != mod.mLastPitchBend)
      {
         mod.mLastPitchBend = bend;
         mDevice.SendPitchBend((int)ofMap(bend,-mPitchBendRange,mPitchBendRange,0,16383,K(clamp)), channel);
      }
      float modWheel = mod.mModulation.modWheel ? mod.mModulation.modWheel->GetValue(0) : 0;
      if (modWheel != mod.mLastModWheel)
      {
         mod.mLastModWheel = modWheel;
         mDevice.SendCC(mModwheelCC, modWheel * 127, channel);
      }
      float pressure = mod.mModulation.pressure ? mod.mModulation.pressure->GetValue(0) : 0;
      if (pressure != mod.mLastPressure)
      {
         mod.mLastPressure = pressure;
         mDevice.SendAftertouch(pressure*127, channel);
      }
   }
}

void MidiOutputModule::DropdownUpdated(DropdownList* list, int oldVal)
{
   mDevice.ConnectOutput(mControllerIndex);
}

void MidiOutputModule::DropdownClicked(DropdownList* list)
{
   BuildControllerList();
}

void MidiOutputModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("controller",moduleInfo,"",FillDropdown<MidiController*>);
   mModuleSaveData.LoadInt("channel",moduleInfo,1,1,16);
   mModuleSaveData.LoadBool("usevoiceaschannel", moduleInfo, false);
   mModuleSaveData.LoadFloat("pitchbendrange",moduleInfo,2,1,96,K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)",moduleInfo,1,0,127,K(isTextField));
   
   SetUpFromSaveData();
}

void MidiOutputModule::SetUpFromSaveData()
{
   InitController();
   
   mChannel = mModuleSaveData.GetInt("channel");
   mUseVoiceAsChannel = mModuleSaveData.GetBool("usevoiceaschannel");
   mPitchBendRange = mModuleSaveData.GetFloat("pitchbendrange");
   mModwheelCC = mModuleSaveData.GetInt("modwheelcc(1or74)");
}
