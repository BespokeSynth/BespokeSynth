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

    VolcaBeatsControl.cpp
    Created: 28 Jan 2017 10:48:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "VolcaBeatsControl.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

VolcaBeatsControl::VolcaBeatsControl()
{
   for (int i = 0; i < 10; ++i)
   {
      mLevels[i] = 1;
      mLevelSliders[i] = nullptr;
   }
}

VolcaBeatsControl::~VolcaBeatsControl()
{
}

void VolcaBeatsControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mClapSpeedSlider = new FloatSlider(this, "clap speed", 5, 5, 140, 15, &mClapSpeed, 0, 1);
   mClaveSpeedSlider = new FloatSlider(this, "clave speed", mClapSpeedSlider, kAnchor_Below, 140, 15, &mClaveSpeed, 0, 1);
   mAgogoSpeedSlider = new FloatSlider(this, "agogo speed", mClaveSpeedSlider, kAnchor_Below, 140, 15, &mAgogoSpeed, 0, 1);
   mCrashSpeedSlider = new FloatSlider(this, "crash speed", mAgogoSpeedSlider, kAnchor_Below, 140, 15, &mCrashSpeed, 0, 1);
   mStutterTimeSlider = new FloatSlider(this, "stutter time", mCrashSpeedSlider, kAnchor_Below, 140, 15, &mStutterTime, 0, 1);
   mStutterDepthSlider = new FloatSlider(this, "stutter depth", mStutterTimeSlider, kAnchor_Below, 140, 15, &mStutterDepth, 0, 1);
   mTomDecaySlider = new FloatSlider(this, "tom decay", mStutterDepthSlider, kAnchor_Below, 140, 15, &mTomDecay, 0, 1);
   mClosedHatDecaySlider = new FloatSlider(this, "closed hat decay", mTomDecaySlider, kAnchor_Below, 140, 15, &mClosedHatDecay, 0, 1);
   mOpenHatDecaySlider = new FloatSlider(this, "open hat decay", mClosedHatDecaySlider, kAnchor_Below, 140, 15, &mOpenHatDecay, 0, 1);
   mHatGrainSlider = new FloatSlider(this, "hat grain", mOpenHatDecaySlider, kAnchor_Below, 140, 15, &mHatGrain, 0, 1);

   for (int i = 0; i < 10; ++i)
   {
      mLevelSliders[i] = new FloatSlider(this, ("level " + ofToString(i)).c_str(), 155, 5, 100, 15, &mLevels[i], 0, 1);
      if (i > 0)
         mLevelSliders[i]->PositionTo(mLevelSliders[i - 1], kAnchor_Below);
   }
}

void VolcaBeatsControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mClapSpeedSlider->Draw();
   mClaveSpeedSlider->Draw();
   mAgogoSpeedSlider->Draw();
   mCrashSpeedSlider->Draw();
   mStutterTimeSlider->Draw();
   mStutterDepthSlider->Draw();
   mTomDecaySlider->Draw();
   mClosedHatDecaySlider->Draw();
   mOpenHatDecaySlider->Draw();
   mHatGrainSlider->Draw();
   for (int i = 0; i < 10; ++i)
      mLevelSliders[i]->Draw();
}

void VolcaBeatsControl::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.pitch < 10)
   {
      mLevelSliders[note.pitch]->Compute();
      note.velocity *= mLevels[note.pitch];
   }

   switch (note.pitch)
   {
      case 0: //kick
         note.pitch = 36;
         if (note.velocity > 0)
            SendCC(40, note.velocity);
         break;
      case 1: //snare
         note.pitch = 38;
         if (note.velocity > 0)
            SendCC(41, note.velocity);
         break;
      case 2: //closed
         note.pitch = 42;
         if (note.velocity > 0)
         {
            SendCC(44, note.velocity);
            mClosedHatDecaySlider->Compute();
            mHatGrainSlider->Compute();
         }
         break;
      case 3: //ride
         note.pitch = 67;
         if (note.velocity > 0)
         {
            SendCC(48, note.velocity);
            mAgogoSpeedSlider->Compute();
         }
         break;
      case 4: //clap
         note.pitch = 39;
         if (note.velocity > 0)
         {
            SendCC(46, note.velocity);
            mClapSpeedSlider->Compute();
         }
         break;
      case 5: //crash
         note.pitch = 49;
         if (note.velocity > 0)
         {
            SendCC(49, note.velocity);
            mCrashSpeedSlider->Compute();
         }
         break;
      case 6: //open
         note.pitch = 46;
         if (note.velocity > 0)
         {
            SendCC(45, note.velocity);
            mOpenHatDecaySlider->Compute();
            mHatGrainSlider->Compute();
         }
         break;
      case 7: //stick
         note.pitch = 75;
         if (note.velocity > 0)
         {
            SendCC(47, note.velocity);
            mClaveSpeedSlider->Compute();
         }
         break;
      case 8: //floor
         note.pitch = 43;
         if (note.velocity > 0)
         {
            SendCC(42, note.velocity);
            mTomDecaySlider->Compute();
         }
         break;
      case 9: //low
         note.pitch = 50;
         if (note.velocity > 0)
         {
            SendCC(43, note.velocity);
            mTomDecaySlider->Compute();
         }
         break;
      default:
         note.pitch = -1;
   }

   mStutterTimeSlider->Compute();
   mStutterDepthSlider->Compute();

   if (note.pitch != -1)
      PlayNoteOutput(note);
}

void VolcaBeatsControl::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mClapSpeedSlider)
      SendCC(50, (int)(mClapSpeed * 127));
   if (slider == mClaveSpeedSlider)
      SendCC(51, (int)(mClaveSpeed * 127));
   if (slider == mAgogoSpeedSlider)
      SendCC(52, (int)(mAgogoSpeed * 127));
   if (slider == mCrashSpeedSlider)
      SendCC(53, (int)(mCrashSpeed * 127));
   if (slider == mStutterTimeSlider)
      SendCC(54, (int)(mStutterTime * 127));
   if (slider == mStutterDepthSlider)
      SendCC(55, (int)(mStutterDepth * 127));
   if (slider == mTomDecaySlider)
      SendCC(56, (int)(mTomDecay * 127));
   if (slider == mClosedHatDecaySlider)
      SendCC(57, (int)(mClosedHatDecay * 127));
   if (slider == mOpenHatDecaySlider)
      SendCC(58, (int)(mOpenHatDecay * 127));
   if (slider == mHatGrainSlider)
      SendCC(59, (int)(mHatGrain * 127));
}

void VolcaBeatsControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void VolcaBeatsControl::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
