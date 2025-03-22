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

    LinnstrumentControl.cpp
    Created: 28 Oct 2018 2:17:18pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "LinnstrumentControl.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "ModulationChain.h"
#include "PolyphonyMgr.h"
#include "MidiController.h"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

LinnstrumentControl::LinnstrumentControl()
: mDevice(this)
{
   TheScale->AddListener(this);

   for (size_t i = 0; i < mGridColorState.size(); ++i)
      mGridColorState[i] = kLinnColor_Invalid;
}

LinnstrumentControl::~LinnstrumentControl()
{
   TheScale->RemoveListener(this);
}

void LinnstrumentControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControllerList = new DropdownList(this, "controller", 5, 5, &mControllerIndex);
   mDecaySlider = new FloatSlider(this, "decay", mControllerList, kAnchor_Below, 100, 15, &mDecayMs, 0, 2000);
   mBlackoutCheckbox = new Checkbox(this, "blackout", mDecaySlider, kAnchor_Below, &mBlackout);
   mLightOctavesCheckbox = new Checkbox(this, "octaves", mBlackoutCheckbox, kAnchor_Below, &mLightOctaves);
   mGuitarLinesCheckbox = new Checkbox(this, "guitar lines", mBlackoutCheckbox, kAnchor_Right, &mGuitarLines);
}

void LinnstrumentControl::Init()
{
   IDrawableModule::Init();

   InitController();
}

void LinnstrumentControl::Poll()
{
   for (int i = 0; i < 128; ++i)
      mNoteAge[i].Update(i, this);

   if (gTime - mRequestedOctaveTime > 2000)
   {
      SendNRPN(299, 36); //request left split octave
      mRequestedOctaveTime = gTime;
   }
}

void LinnstrumentControl::InitController()
{
   BuildControllerList();

   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
   {
      if (strstr(devices[i].c_str(), "LinnStrument") != nullptr)
      {
         mControllerIndex = i;
         mDevice.ConnectOutput(i);
         mDevice.ConnectInput(devices[i].c_str());
         UpdateScaleDisplay();
         break;
      }
   }
}

void LinnstrumentControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mControllerList->Draw();
   mDecaySlider->Draw();
   mBlackoutCheckbox->Draw();
   mLightOctavesCheckbox->Draw();
   mGuitarLinesCheckbox->Draw();
}

void LinnstrumentControl::BuildControllerList()
{
   mControllerList->Clear();
   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
      mControllerList->AddLabel(devices[i].c_str(), i);
}

void LinnstrumentControl::OnScaleChanged()
{
   UpdateScaleDisplay();
}

void LinnstrumentControl::UpdateScaleDisplay()
{
   //SendScaleInfo();

   for (int y = 0; y < kRows; ++y)
   {
      mDevice.SendCC(21, y); //only set row once, to cut down on messages
      for (int x = 0; x < kCols; ++x)
      {
         SetGridColor(x, y, GetDesiredGridColor(x, y), true);
      }
   }
}

void LinnstrumentControl::SetGridColor(int x, int y, LinnstrumentColor color, bool ignoreRow /*= false*/)
{
   if (x < kCols && y < kRows)
   {
      if (color != mGridColorState[x + y * kCols])
      {
         if (!ignoreRow)
            mDevice.SendCC(21, y);
         mDevice.SendCC(20, x + 1);
         mDevice.SendCC(22, color);
         mGridColorState[x + y * kCols] = color;
      }
   }
}

LinnstrumentControl::LinnstrumentColor LinnstrumentControl::GetDesiredGridColor(int x, int y)
{
   if (mBlackout)
      return kLinnColor_Black;

   if (mGuitarLines)
   {
      if (x % 2 == 1)
      {
         /*LinnstrumentColor darkColor = kLinnColor_Green;
         LinnstrumentColor lightColor = kLinnColor_Lime;
         
         if (x % 6 == 1)
         {
            darkColor = kLinnColor_Blue;
            lightColor = kLinnColor_Cyan;
         }
         
         if (x % 4 == 1)
         {
            if (y % 4 < 2)
               return darkColor;
            return lightColor;
         }
         else
         {
            if (y % 4 < 2)
               return lightColor;
            return darkColor;
         }*/
         LinnstrumentColor colors[4];
         colors[0] = kLinnColor_Lime;
         colors[1] = kLinnColor_Green;
         colors[2] = kLinnColor_Cyan;
         colors[3] = kLinnColor_Blue;

         int horizontalIndex = x / 2;
         int verticalIndex = (7 - y) / 2;

         return colors[(horizontalIndex + verticalIndex) % 4];
      }
      else
      {
         return kLinnColor_Black;
      }
   }

   int pitch = GridToPitch(x, y);
   if (TheScale->IsRoot(pitch))
      return kLinnColor_Green;
   if (TheScale->IsInPentatonic(pitch))
      return kLinnColor_Orange;
   if (TheScale->IsInScale(pitch))
      return kLinnColor_Yellow;
   return kLinnColor_Black;
}

int LinnstrumentControl::GridToPitch(int x, int y)
{
   return 30 + x + y * 5 + (mLinnstrumentOctave - 5) * 12;
}

void LinnstrumentControl::SetPitchColor(int pitch, LinnstrumentColor color)
{
   for (int y = 0; y < kRows; ++y)
   {
      for (int x = 0; x < kCols; ++x)
      {
         if (GridToPitch(x, y) == pitch)
            SetGridColor(x, y, (color == kLinnColor_Off) ? GetDesiredGridColor(x, y) : color);
      }
   }
}

void LinnstrumentControl::SendScaleInfo()
{
   /*NRPN format:
   send these 6 messages in a row:
   1011nnnn   01100011 ( 99)  0vvvvvvv         NRPN parameter number MSB CC
   1011nnnn   01100010 ( 98)  0vvvvvvv         NRPN parameter number LSB CC
   1011nnnn   00000110 (  6)  0vvvvvvv         NRPN parameter value MSB CC
   1011nnnn   00100110 ( 38)  0vvvvvvv         NRPN parameter value LSB CC
   1011nnnn   01100101 (101)  01111111 (127)   RPN parameter number Reset MSB CC
   1011nnnn   01100100 (100)  01111111 (127)   RPN parameter number Reset LSB CC
   
   Linnstrument NRPN scale messages:
   203    0-1     Global Main Note Light C (0: Off, 1: On)
   204    0-1     Global Main Note Light C# (0: Off, 1: On)
   205    0-1     Global Main Note Light D (0: Off, 1: On)
   206    0-1     Global Main Note Light D# (0: Off, 1: On)
   207    0-1     Global Main Note Light E (0: Off, 1: On)
   208    0-1     Global Main Note Light F (0: Off, 1: On)
   209    0-1     Global Main Note Light F# (0: Off, 1: On)
   210    0-1     Global Main Note Light G (0: Off, 1: On)
   211    0-1     Global Main Note Light G# (0: Off, 1: On)
   212    0-1     Global Main Note Light A (0: Off, 1: On)
   213    0-1     Global Main Note Light A# (0: Off, 1: On)
   214    0-1     Global Main Note Light B (0: Off, 1: On)
   215    0-1     Global Accent Note Light C (0: Off, 1: On)
   216    0-1     Global Accent Note Light C# (0: Off, 1: On)
   217    0-1     Global Accent Note Light D (0: Off, 1: On)
   218    0-1     Global Accent Note Light D# (0: Off, 1: On)
   219    0-1     Global Accent Note Light E (0: Off, 1: On)
   220    0-1     Global Accent Note Light F (0: Off, 1: On)
   221    0-1     Global Accent Note Light F# (0: Off, 1: On)
   222    0-1     Global Accent Note Light G (0: Off, 1: On)
   223    0-1     Global Accent Note Light G# (0: Off, 1: On)
   224    0-1     Global Accent Note Light A (0: Off, 1: On)
   225    0-1     Global Accent Note Light A# (0: Off, 1: On)
   226    0-1     Global Accent Note Light B (0: Off, 1: On)
   */

   const unsigned char setMainNoteBase = 203;
   const unsigned char setAccentNoteBase = 215;
   if (TheScale->GetPitchesPerOctave() == 12) //linnstrument only works with 12-tet
   {
      for (int pitch = 0; pitch < 12; ++pitch)
      {
         //set main note
         int number = setMainNoteBase + pitch;
         SendNRPN(number, TheScale->IsInScale(pitch));

         std::this_thread::sleep_for(10ms);

         //set accent note
         number = setAccentNoteBase + pitch;
         SendNRPN(number, TheScale->IsRoot(pitch));

         std::this_thread::sleep_for(10ms);
      }
   }
}

void LinnstrumentControl::SendNRPN(int param, int value)
{
   const unsigned char channelHeader = 177;
   mDevice.SendData(channelHeader, 99, param >> 7);
   mDevice.SendData(channelHeader, 98, param & 0x7f);
   mDevice.SendData(channelHeader, 6, value >> 7);
   mDevice.SendData(channelHeader, 38, value & 0x7f);
   mDevice.SendData(channelHeader, 101, 127);
   mDevice.SendData(channelHeader, 100, 127);
}

void LinnstrumentControl::PlayNote(NoteMessage note)
{
   if (note.voiceIdx == -1)
      note.voiceIdx = 0;

   mModulators[note.voiceIdx] = note.modulation;

   if (note.pitch >= 0 && note.pitch < 128)
   {
      mNoteAge[note.pitch].mTime = note.velocity > 0 ? -1 : note.time;
      if (note.velocity > 0)
         mNoteAge[note.pitch].mVoiceIndex = note.voiceIdx;
      mNoteAge[note.pitch].Update(note.pitch, this);
   }
}

void LinnstrumentControl::OnMidiNote(MidiNote& note)
{
   if (mControlPlayedLights)
      mDevice.SendNote(gTime, note.mPitch, 0, false, note.mChannel); //don't allow linnstrument to light played notes
}

void LinnstrumentControl::OnMidiControl(MidiControl& control)
{
   if (control.mControl == 101)
      mLastReceivedNRPNParamMSB = control.mValue;
   if (control.mControl == 100)
      mLastReceivedNRPNParamLSB = control.mValue;
   if (control.mControl == 6)
      mLastReceivedNRPNValueMSB = control.mValue;
   if (control.mControl == 38)
   {
      mLastReceivedNRPNValueLSB = control.mValue;

      int nrpnParam = mLastReceivedNRPNParamMSB << 7 | mLastReceivedNRPNParamLSB;
      int nrpnValue = mLastReceivedNRPNValueMSB << 7 | mLastReceivedNRPNValueLSB;

      if (nrpnParam == 36)
         mLinnstrumentOctave = nrpnValue;
   }
}

void LinnstrumentControl::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mControllerList)
   {
      mDevice.ConnectOutput(mControllerIndex);
      UpdateScaleDisplay();
   }
}

void LinnstrumentControl::DropdownClicked(DropdownList* list)
{
   BuildControllerList();
}

void LinnstrumentControl::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mBlackoutCheckbox)
   {
      if (mBlackout)
         mGuitarLines = false;
      UpdateScaleDisplay();
   }

   if (checkbox == mGuitarLinesCheckbox)
   {
      if (mGuitarLines)
         mBlackout = false;
      UpdateScaleDisplay();
   }
}

void LinnstrumentControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void LinnstrumentControl::SetUpFromSaveData()
{
   InitController();
}

void LinnstrumentControl::NoteAge::Update(int pitch, LinnstrumentControl* linnstrument)
{
   if (!linnstrument->mControlPlayedLights)
      return;

   float age = gTime - mTime;
   LinnstrumentColor newColor;
   if (mTime < 0 || age < 0)
      newColor = kLinnColor_Red;
   else if (age < linnstrument->mDecayMs * .25f)
      newColor = kLinnColor_Pink;
   else if (age < linnstrument->mDecayMs * .5f)
      newColor = kLinnColor_Cyan;
   else if (age < linnstrument->mDecayMs)
      newColor = kLinnColor_Blue;
   else
      newColor = kLinnColor_Off;

   if (mVoiceIndex != -1 && linnstrument->mModulators[mVoiceIndex].pitchBend)
   {
      float bendRounded = round(linnstrument->mModulators[mVoiceIndex].pitchBend->GetValue(0));
      pitch += (int)bendRounded;
   }

   if (newColor != mColor || pitch != mOutputPitch)
   {
      for (int i = -2; i <= 2; ++i)
      {
         if (i != 0 && !linnstrument->mLightOctaves)
            continue;

         LinnstrumentColor color = newColor;

         if (i != 0)
         {
            if (mTime < 0 || age < 0)
               color = kLinnColor_Pink;
            else
               color = kLinnColor_Off;
         }

         if (pitch != mOutputPitch)
            linnstrument->SetPitchColor(mOutputPitch + i * TheScale->GetPitchesPerOctave(), kLinnColor_Off);
         linnstrument->SetPitchColor(pitch + i * TheScale->GetPitchesPerOctave(), color);
      }
      mColor = newColor;
      mOutputPitch = pitch;
   }
}
