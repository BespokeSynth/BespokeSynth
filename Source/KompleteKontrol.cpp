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
//  KompleteKontrol.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/11/15.
//
//

#if BESPOKE_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "KompleteKontrol.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "MidiController.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

KompleteKontrol::KompleteKontrol()
: mInitialized(false)
, mKeyOffset(36)
, mNeedKeysUpdate(false)
, mController(nullptr)
{
   mKontrol.Init();
   mKontrol.QueueMessage("NIHWMainHandler", mKontrol.CreateMessage("NIGetServiceVersionMessage"));
   TheScale->AddListener(this);
   mKontrol.SetListener(this);
}

void KompleteKontrol::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mMidiControllerCable = new PatchCableSource(this, kConnectionType_Special);
   mMidiControllerCable->AddTypeFilter("midicontroller");
   mMidiControllerCable->SetManualPosition(95, 10);
   AddPatchCableSource(mMidiControllerCable);
}

KompleteKontrol::~KompleteKontrol()
{
   TheScale->RemoveListener(this);
}

void KompleteKontrol::Exit()
{
   IDrawableModule::Exit();
   mKontrol.Exit();
}

void KompleteKontrol::Poll()
{
   mKontrol.Update();

   if (!mInitialized && mKontrol.IsReady())
   {
      mNeedKeysUpdate = true;
      mInitialized = true;
   }

   if (mNeedKeysUpdate)
      UpdateKeys();

   for (int i = 0; i < 9; ++i)
   {
      int control = i - 1 + 100;
      if (i == 0)
         control = 21;
      UIControlConnection* connection = nullptr;
      if (mController)
         connection = mController->GetConnectionForControl(kMidiMessage_Control, control);
      if (connection)
      {
         IUIControl* uicontrol = connection->GetUIControl();
         if (uicontrol)
         {
            mTextBoxes[i].slider = true;
            mTextBoxes[i].amount = uicontrol->GetMidiValue();
            mTextBoxes[i].line1 = juce::String(uicontrol->Name()).toUpperCase().toStdString();
            mTextBoxes[i].line2 = juce::String(uicontrol->GetDisplayValue(uicontrol->GetValue())).toUpperCase().toStdString();
            if (mTextBoxes[i].line2.length() > 0 && mTextBoxes[i].line2[0] == '.')
               mTextBoxes[i].line2 = " " + mTextBoxes[i].line2; //can't have a period as the first character, so add a space
         }
         else
         {
            mTextBoxes[i].slider = false;
            mTextBoxes[i].line1 = "";
            mTextBoxes[i].line2 = "";
         }
      }
      else
      {
         mTextBoxes[i].slider = false;
         mTextBoxes[i].line1 = "";
         mTextBoxes[i].line2 = "";
      }
   }

   if (mInitialized)
      UpdateText();
}

void KompleteKontrol::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawTextNormal("midicontroller:", 5, 13);
}

void KompleteKontrol::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mMidiControllerCable)
   {
      mController = dynamic_cast<MidiController*>(mMidiControllerCable->GetTarget());
   }
}

void KompleteKontrol::PlayNote(NoteMessage note)
{
   mNoteOutput.PlayNote(note);

   mNeedKeysUpdate = true;
}

void KompleteKontrol::OnScaleChanged()
{
   mNeedKeysUpdate = true;
}

void KompleteKontrol::UpdateKeys()
{
   if (!mKontrol.IsReady())
      return;

   ofColor keys[61];
   for (int i = 0; i < kNumKeys; ++i)
   {
      int pitch = i + mKeyOffset;
      bool inScale = TheScale->MakeDiatonic(pitch) == pitch;
      bool isRoot = pitch % TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot();
      bool isFifth = (pitch - 7) % TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot();
      bool isHeld = false;
      bool isInPentatonic = pitch >= 0 && TheScale->IsInPentatonic(pitch);

      std::list<int> heldNotes = mNoteOutput.GetHeldNotesList();
      for (int iter : heldNotes)
      {
         if (iter == pitch)
            isHeld = true;
      }

      if (isRoot)
         keys[i] = ofColor(0, 255, 0);
      else if (TheScale->GetPitchesPerOctave() != 12)
         keys[i] = ofColor(40, 15, 0);
      else if (isFifth && inScale)
         keys[i] = ofColor(255, 0, 70);
      else if (isInPentatonic)
         keys[i] = ofColor(255, 70, 0);
      else if (inScale)
         keys[i] = ofColor(40, 15, 0);
      else
         keys[i] = ofColor::black;

      if (isHeld)
      {
         if (keys[i].r == 0 && keys[i].g == 0 && keys[i].b == 0)
            keys[i] = ofColor::blue; //out-of-key pressed notes turn blue
         else
            keys[i] = ofColor::red;
      }
   }
   mKontrol.SetKeyLights(keys);
}

void KompleteKontrol::UpdateText()
{
   if (mKontrol.IsReady() == false)
      return;

   uint16_t sliders[NUM_SLIDER_SEGMENTS];
   std::string text;
   for (int i = 0; i < 9; ++i)
   {
      int numPeriods1 = 0;
      int numPeriods2 = 0;
      for (int j = 0; j < 8; ++j)
      {
         uint16_t cell = 0;
         if (mTextBoxes[i].slider)
         {
            if (j == 0)
               cell |= 4;
            if (mTextBoxes[i].amount > j / 8.0f)
               cell |= 3;
            if (j == 7 && mTextBoxes[i].amount > .999f)
               cell |= 0xffff; //fill that ninth cell
         }

         if (j + 1 + numPeriods1 < mTextBoxes[i].line1.length() &&
             mTextBoxes[i].line1[j + 1 + numPeriods1] == '.')
         {
            ++numPeriods1;
            cell |= 256;
         }
         if (j + 1 + numPeriods2 < mTextBoxes[i].line2.length() &&
             mTextBoxes[i].line2[j + 1 + numPeriods2] == '.')
         {
            ++numPeriods2;
            cell |= 512;
         }

         sliders[i * 8 + j] = cell;
      }
   }
   for (int i = 0; i < 9; ++i)
   {
      ofStringReplace(mTextBoxes[i].line1, ".", "");
      for (int j = 0; j < 8; ++j)
      {
         if (j < mTextBoxes[i].line1.length())
            text += mTextBoxes[i].line1[j];
         else
            text += " ";
      }
   }
   for (int i = 0; i < 9; ++i)
   {
      ofStringReplace(mTextBoxes[i].line2, ".", "");
      for (int j = 0; j < 8; ++j)
      {
         if (j < mTextBoxes[i].line2.length())
            text += mTextBoxes[i].line2[j];
         else
            text += " ";
      }
   }

   if (text != mCurrentText ||
       memcmp(sliders, mCurrentSliders, NUM_SLIDER_SEGMENTS * sizeof(uint16_t)) != 0)
   {
      mKontrol.SetDisplay(sliders, text);
      mCurrentText = text;
      memcpy(mCurrentSliders, sliders, NUM_SLIDER_SEGMENTS * sizeof(uint16_t));
   }
}

void KompleteKontrol::OnKontrolButton(int control, bool on)
{
   if (mController)
   {
      if (control >= 24 && control <= 32) //the touch-sensitive encoder buttons
      {
         if (on)
         {
            int associatedControl = control + 76; //encoders are 100-108
            UIControlConnection* connection = mController->GetConnectionForControl(kMidiMessage_Control, associatedControl);
            if (connection)
            {
               IUIControl* uicontrol = connection->GetUIControl();
               if (uicontrol)
                  uicontrol->StartBeacon();
            }
         }
      }
      else
      {
         if (control == 1) //fix collision with mod wheel
            control = 98;

         MidiControl c;
         c.mDeviceName = 0;
         c.mChannel = 1;
         c.mControl = control;
         c.mValue = on ? 127 : 0;
         mController->OnMidiControl(c);
      }
   }
}

void KompleteKontrol::OnKontrolEncoder(int control, float change)
{
   if (mController)
   {
      MidiControl c;
      c.mDeviceName = 0;
      c.mChannel = 1;
      c.mControl = control + 100;
      c.mValue = change > 0 ? 127 : 0;
      mController->OnMidiControl(c);
   }
}

void KompleteKontrol::OnKontrolOctave(int octave)
{
   mKeyOffset = octave;
   mNeedKeysUpdate = true;
}

void KompleteKontrol::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);

   SetUpFromSaveData();
}

void KompleteKontrol::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mMidiControllerCable->SetTarget(TheSynth->FindMidiController(mModuleSaveData.GetString("controller")));
}
