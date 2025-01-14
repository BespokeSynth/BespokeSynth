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
//  VelocityCurve.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/17/22.
//
//

#include "VelocityCurve.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

namespace
{
   const int kAdsrTime = 10000;
}

VelocityCurve::VelocityCurve()
{
   mEnvelopeControl.SetADSR(&mAdsr);
   mEnvelopeControl.SetViewLength(kAdsrTime);
   mEnvelopeControl.SetFixedLengthMode(true);
   mAdsr.GetFreeReleaseLevel() = true;
   mAdsr.SetNumStages(2);
   mAdsr.GetHasSustainStage() = false;
   mAdsr.GetStageData(0).target = 0;
   mAdsr.GetStageData(0).time = 0.01f;
   mAdsr.GetStageData(1).target = 1;
   mAdsr.GetStageData(1).time = kAdsrTime - .02f;
}

void VelocityCurve::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void VelocityCurve::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mEnvelopeControl.Draw();

   const double kDisplayInputMs = 400;
   if (gTime < mLastInputTime + kDisplayInputMs)
   {
      float pos = mLastInputVelocity / 127.0f * mEnvelopeControl.GetDimensions().x + mEnvelopeControl.GetPosition().x;
      ofPushStyle();
      ofSetColor(0, 255, 0, (1 - (gTime - mLastInputTime) / kDisplayInputMs) * 255);
      ofLine(pos, mEnvelopeControl.GetPosition().y, pos, mEnvelopeControl.GetPosition().y + mEnvelopeControl.GetDimensions().y);
      ofPopStyle();
   }
}

void VelocityCurve::PlayNote(NoteMessage note)
{
   if (mEnabled)
   {
      if (note.velocity > 0)
      {
         mLastInputVelocity = note.velocity;
         mLastInputTime = note.time;

         ComputeSliders(0);
         ADSR::EventInfo adsrEvent(0, kAdsrTime);
         float val = ofClamp(mAdsr.Value(note.velocity / 127.0f * kAdsrTime, &adsrEvent), 0, 1);
         if (std::isnan(val))
            val = 0;
         note.velocity = val * 127;
         if (note.velocity <= 0)
            note.velocity = 1;
      }
   }

   PlayNoteOutput(note);
}

void VelocityCurve::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mEnvelopeControl.OnClicked(x, y, right);
}

void VelocityCurve::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mEnvelopeControl.MouseReleased();
}

bool VelocityCurve::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mEnvelopeControl.MouseMoved(x, y);

   return false;
}

void VelocityCurve::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VelocityCurve::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void VelocityCurve::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mAdsr.SaveState(out);
}

void VelocityCurve::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mAdsr.LoadState(in);
}
