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
//  VelocityToDuration.cpp
//  Bespoke
//
//  Created by Andrius Merkys on 9/15/25.
//
//

#include "VelocityToDuration.h"
#include "SynthGlobals.h"

VelocityToDuration::VelocityToDuration()
{
}

void VelocityToDuration::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void VelocityToDuration::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMaxDurationSlider = new FloatSlider(this, "max_duration", 5, 2, 100, 15, &mMaxDuration, 0.01f, 4, 4);
   mMaxDurationSlider->SetMode(FloatSlider::kSquare);
}

VelocityToDuration::~VelocityToDuration()
{
   TheTransport->RemoveAudioPoller(this);
}

void VelocityToDuration::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMaxDurationSlider->Draw();
}

void VelocityToDuration::OnTransportAdvanced(float amount)
{
   for (auto iter = mNoteOffs.begin(); iter != mNoteOffs.end();)
   {
      if (iter->mTime < gTime)
      {
         PlayNoteOutput(gTime, iter->mPitch, 0, iter->mVoiceIdx);
         iter = mNoteOffs.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
}

void VelocityToDuration::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }

   if (velocity > 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (velocity > 0)
   {
      ComputeSliders(0);

      float durationMs = velocity / 127 * mMaxDuration / (float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom()) * TheTransport->MsPerBar();

      bool found = false;
      for (auto& queued : mNoteOffs)
      {
         if (queued.mPitch == pitch)
         {
            queued.mTime = time + durationMs;
            queued.mVoiceIdx = voiceIdx;
            found = true;
            break;
         }
      }

      if (!found)
         mNoteOffs.push_back(QueuedNoteOff(time + durationMs, pitch, voiceIdx));
   }
}

void VelocityToDuration::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void VelocityToDuration::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VelocityToDuration::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
