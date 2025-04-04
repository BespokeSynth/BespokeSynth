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
//  Arpeggiator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "Arpeggiator.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

Arpeggiator::Arpeggiator()
{
   TheScale->AddListener(this);
}

void Arpeggiator::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void Arpeggiator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 20, 140);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 50);
   INTSLIDER(mArpStepSlider, "step", &mArpStep, -3, 3);
   INTSLIDER(mOctaveRepeatsSlider, "octaves", &mOctaveRepeats, 1, 4);
   ENDUIBLOCK(mWidth, mHeight);

   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
}

Arpeggiator::~Arpeggiator()
{
   TheTransport->RemoveListener(this);
   TheScale->RemoveListener(this);
}

void Arpeggiator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofSetColor(255, 255, 255, gModuleDrawAlpha);

   mIntervalSelector->Draw();
   mArpStepSlider->Draw();
   mOctaveRepeatsSlider->Draw();

   ofSetColor(200, 200, 200, gModuleDrawAlpha);
   std::string chord;
   for (int i = 0; i < mChord.size(); ++i)
      chord += GetArpNoteDisplay(mChord[i].pitch) + " ";
   DrawTextNormal(chord, 5, 16);
   ofSetColor(0, 255, 0, gModuleDrawAlpha);
   std::string pad;
   for (int i = 0; i < mChord.size(); ++i)
   {
      if (i != mArpIndex)
      {
         pad += GetArpNoteDisplay(mChord[i].pitch) + " ";
      }
      else
      {
         float w = gFont.GetStringWidth(pad, 13);
         DrawTextNormal(GetArpNoteDisplay(mChord[i].pitch), 6 + w, 16);
         break;
      }
   }
}

void Arpeggiator::OnScaleChanged()
{
   mChordMutex.lock();
   mChord.clear();
   mChordMutex.unlock();
}

std::string Arpeggiator::GetArpNoteDisplay(int pitch)
{
   return NoteName(pitch, false, true);
}

void Arpeggiator::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void Arpeggiator::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool Arpeggiator::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   return false;
}

void Arpeggiator::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mChordMutex.lock();
      mChord.clear();
      mChordMutex.unlock();

      mNoteOutput.Flush(time);
   }
}

void Arpeggiator::PlayNote(NoteMessage note)
{
   if (!mEnabled || note.pitch < 0 || note.pitch >= 128)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.velocity > 0 && !mInputNotes[note.pitch])
   {
      mChordMutex.lock();
      mChord.push_back(ArpNote(note.pitch, note.velocity, note.voiceIdx, note.modulation));
      mChordMutex.unlock();
   }

   if (note.velocity == 0 && mInputNotes[note.pitch])
   {
      mChordMutex.lock();
      for (auto iter = mChord.begin(); iter != mChord.end(); ++iter)
      {
         if (iter->pitch == note.pitch)
         {
            mChord.erase(iter);
            break;
         }
      }
      mChordMutex.unlock();
   }

   mInputNotes[note.pitch] = note.velocity > 0;
}

void Arpeggiator::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   if (mChord.size() == 0)
   {
      if (mLastPitch != -1)
      {
         NoteMessage note(time, mLastPitch, 0);
         PlayNoteOutput(note);
      }
      mLastPitch = -1;
      return;
   }

   if (mArpStep != 0)
   {
      mArpIndex += mArpStep;
      if (mChord.size() > 0)
      {
         while (mArpIndex >= (int)mChord.size())
         {
            mArpIndex -= mChord.size();
            mCurrentOctaveOffset = (mCurrentOctaveOffset + 1) % mOctaveRepeats;
         }
         while (mArpIndex < 0)
         {
            mArpIndex += mChord.size();
            mCurrentOctaveOffset = (mCurrentOctaveOffset - 1 + mOctaveRepeats) % mOctaveRepeats;
         }
      }
   }
   else //pingpong
   {
      assert(mArpPingPongDirection == 1 || mArpPingPongDirection == -1);
      mArpIndex += mArpPingPongDirection;
      if (mChord.size() >= 2)
      {
         if (mArpIndex < 0)
         {
            mArpIndex = 1;
            mArpPingPongDirection = 1;
         }
         if (mArpIndex > mChord.size() - 1)
         {
            mArpIndex = (int)mChord.size() - 2;
            mArpPingPongDirection = -1;
         }
      }
      else
      {
         mArpIndex = ofClamp(mArpIndex, 0, mChord.size() - 1);
      }
   }

   int offPitch = -1;
   if (mLastPitch >= 0)
   {
      offPitch = mLastPitch;
   }
   if (mChord.size())
   {
      ArpNote current = mChord[mArpIndex];
      int outPitch = current.pitch;

      outPitch += mCurrentOctaveOffset * TheScale->GetPitchesPerOctave();


      if (mLastPitch == outPitch) //same note, play noteoff first
      {
         PlayNoteOutput(NoteMessage(time, mLastPitch, 0));
         offPitch = -1;
      }
      float pressure = current.modulation.pressure ? current.modulation.pressure->GetValue(0) : 0;
      PlayNoteOutput(NoteMessage(time, outPitch, ofClamp(current.vel + 127 * pressure, 0, 127), current.voiceIdx, current.modulation));
      mLastPitch = outPitch;
   }
   if (offPitch != -1)
   {
      PlayNoteOutput(NoteMessage(time, offPitch, 0));
      if (offPitch == mLastPitch)
         mLastPitch = -1;
   }
}

void Arpeggiator::UpdateInterval()
{
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
      transportListenerInfo->mInterval = mInterval;
}

void Arpeggiator::ButtonClicked(ClickButton* button, double time)
{
}

void Arpeggiator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      UpdateInterval();
}

void Arpeggiator::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mArpStepSlider)
   {
      if (oldVal > 0)
         mArpPingPongDirection = 1;
      else if (oldVal < 0)
         mArpPingPongDirection = -1;
   }
}

void Arpeggiator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Arpeggiator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
