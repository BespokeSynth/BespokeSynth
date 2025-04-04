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
//  MidiOutput.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/24/15.
//
//

#include "MidiOutput.h"
#include "SynthGlobals.h"
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
{
   mChannelModulations.resize(kGlobalModulationIdx + 1);
}

MidiOutputModule::~MidiOutputModule()
{
   TheTransport->RemoveAudioPoller(this);
}

void MidiOutputModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControllerList = new DropdownList(this, "controller", 5, 5, &mControllerIndex);
}

void MidiOutputModule::Init()
{
   IDrawableModule::Init();

   InitController();

   TheTransport->AddAudioPoller(this);
}

void MidiOutputModule::InitController()
{
   MidiController* controller = TheSynth->FindMidiController(mModuleSaveData.GetString("controller"));
   if (controller)
      mDevice.ConnectOutput(controller->GetDeviceOut().c_str(), mModuleSaveData.GetInt("channel"));

   BuildControllerList();

   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
   {
      if (strcmp(devices[i].c_str(), mDevice.Name()) == 0)
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
   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
      mControllerList->AddLabel(devices[i].c_str(), i);
}

void MidiOutputModule::PlayNote(NoteMessage note)
{
   int channel = note.voiceIdx + 1;
   if (note.voiceIdx == -1)
      channel = 1;
   channel = mUseVoiceAsChannel ? channel : mChannel;

   mDevice.SendNote(note.time, note.pitch, note.velocity, false, channel);

   int modIdx = channel - 1;
   if (note.voiceIdx == -1)
      modIdx = kGlobalModulationIdx;

   mChannelModulations[modIdx].mModulation = note.modulation;
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
   for (int i = 0; i < mChannelModulations.size(); ++i)
   {
      ChannelModulations& mod = mChannelModulations[i];
      int channel = i + 1;
      if (i == kGlobalModulationIdx)
         channel = 1;
      float bend = mod.mModulation.pitchBend ? mod.mModulation.pitchBend->GetValue(0) : 0;
      if (bend != mod.mLastPitchBend)
      {
         mod.mLastPitchBend = bend;
         mDevice.SendPitchBend((int)ofMap(bend, -mPitchBendRange, mPitchBendRange, 0, 16383, K(clamp)), channel);
      }
      float modWheel = mod.mModulation.modWheel ? mod.mModulation.modWheel->GetValue(0) : ModulationParameters::kDefaultModWheel;
      if (modWheel != mod.mLastModWheel)
      {
         mod.mLastModWheel = modWheel;
         mDevice.SendCC(mModwheelCC, modWheel * 127, channel);
      }
      float pressure = mod.mModulation.pressure ? mod.mModulation.pressure->GetValue(0) : ModulationParameters::kDefaultPressure;
      if (pressure != mod.mLastPressure)
      {
         mod.mLastPressure = pressure;
         mDevice.SendAftertouch(pressure * 127, channel);
      }
   }
}

void MidiOutputModule::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   mDevice.ConnectOutput(mControllerIndex);
}

void MidiOutputModule::DropdownClicked(DropdownList* list)
{
   BuildControllerList();
}

void MidiOutputModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);
   mModuleSaveData.LoadInt("channel", moduleInfo, 1, 1, 16);
   mModuleSaveData.LoadBool("usevoiceaschannel", moduleInfo, false);
   mModuleSaveData.LoadFloat("pitchbendrange", moduleInfo, 2, 1, 96, K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)", moduleInfo, 1, 0, 127, K(isTextField));

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
