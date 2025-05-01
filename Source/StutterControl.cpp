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
//  StutterControl.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/25/15.
//
//

#include "StutterControl.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

StutterControl::StutterControl()
: IAudioProcessor(gBufferSize)
{
   for (int i = 0; i < kNumStutterTypes; ++i)
      mStutter[i] = false;
}

void StutterControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mGridControlTarget = new GridControlTarget(this, "grid", 5, 145);

   mStutterCheckboxes[kHalf] = new Checkbox(this, "half note", 4, 3, &mStutter[kHalf]);
   mStutterCheckboxes[kQuarter] = new Checkbox(this, "quarter", mStutterCheckboxes[kHalf], kAnchor_Below, &mStutter[kQuarter]);
   mStutterCheckboxes[k8th] = new Checkbox(this, "8th", mStutterCheckboxes[kQuarter], kAnchor_Below, &mStutter[k8th]);
   mStutterCheckboxes[k16th] = new Checkbox(this, "16th", mStutterCheckboxes[k8th], kAnchor_Below, &mStutter[k16th]);
   mStutterCheckboxes[k32nd] = new Checkbox(this, "32nd", mStutterCheckboxes[k16th], kAnchor_Below, &mStutter[k32nd]);
   mStutterCheckboxes[k64th] = new Checkbox(this, "64th", mStutterCheckboxes[k32nd], kAnchor_Below, &mStutter[k64th]);
   mStutterCheckboxes[kReverse] = new Checkbox(this, "reverse", mStutterCheckboxes[k64th], kAnchor_Below, &mStutter[kReverse]);
   mStutterCheckboxes[kRampIn] = new Checkbox(this, "ramp in", mStutterCheckboxes[kReverse], kAnchor_Below, &mStutter[kRampIn]);
   mStutterCheckboxes[kRampOut] = new Checkbox(this, "ramp out", mStutterCheckboxes[kHalf], kAnchor_Right, &mStutter[kRampOut]);
   mStutterCheckboxes[kTumbleUp] = new Checkbox(this, "tumble up", mStutterCheckboxes[kRampOut], kAnchor_Below, &mStutter[kTumbleUp]);
   mStutterCheckboxes[kTumbleDown] = new Checkbox(this, "tumble down", mStutterCheckboxes[kTumbleUp], kAnchor_Below, &mStutter[kTumbleDown]);
   mStutterCheckboxes[kHalfSpeed] = new Checkbox(this, "half speed", mStutterCheckboxes[kTumbleDown], kAnchor_Below, &mStutter[kHalfSpeed]);
   mStutterCheckboxes[kDoubleSpeed] = new Checkbox(this, "double speed", mStutterCheckboxes[kHalfSpeed], kAnchor_Below, &mStutter[kDoubleSpeed]);
   mStutterCheckboxes[kDotted8th] = new Checkbox(this, "dotted eighth", mStutterCheckboxes[kDoubleSpeed], kAnchor_Below, &mStutter[kDotted8th]);
   mStutterCheckboxes[kQuarterTriplets] = new Checkbox(this, "triplets", mStutterCheckboxes[kDotted8th], kAnchor_Below, &mStutter[kQuarterTriplets]);
   mStutterCheckboxes[kFree] = new Checkbox(this, "free", mStutterCheckboxes[kQuarterTriplets], kAnchor_Below, &mStutter[kFree]);
   mFreeLengthSlider = new FloatSlider(this, "free length", mStutterCheckboxes[kFree], kAnchor_Below, 102, 15, &mStutterProcessor.mFreeStutterLength, .005f, .25f);
   mFreeSpeedSlider = new FloatSlider(this, "free speed", mFreeLengthSlider, kAnchor_Below, 102, 15, &mStutterProcessor.mFreeStutterSpeed, 0, 2);
}

void StutterControl::Init()
{
   IDrawableModule::Init();

   mStutterProcessor.Init();
}

StutterControl::~StutterControl()
{
}

void StutterControl::Process(double time)
{
   PROFILER(StutterControl);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   IAudioReceiver* target = GetTarget();

   if (target)
   {
      mStutterProcessor.ProcessAudio(time, GetBuffer());

      ChannelBuffer* out = target->GetBuffer();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void StutterControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mStutterProcessor.DrawStutterBuffer(4, 3, 90, 35);

   for (int i = 0; i < kNumStutterTypes; ++i)
      mStutterCheckboxes[i]->Draw();
   mFreeLengthSlider->Draw();
   mFreeSpeedSlider->Draw();
   mGridControlTarget->Draw();
}

void StutterControl::SendStutter(double time, StutterParams stutter, bool on)
{
   if (on)
      mStutterProcessor.StartStutter(time, stutter);
   else
      mStutterProcessor.EndStutter(time, stutter);
   UpdateGridLights();
}

StutterControl::StutterType StutterControl::GetStutterFromKey(int key)
{
   if (key == 'q')
      return kHalf;
   if (key == 'w')
      return kQuarter;
   if (key == 'e')
      return k8th;
   if (key == 'r')
      return k16th;
   if (key == 't')
      return k32nd;
   if (key == 'y')
      return k64th;
   if (key == 'u')
      return kReverse;
   if (key == 'a')
      return kRampIn;
   if (key == 's')
      return kRampOut;
   if (key == 'd')
      return kTumbleUp;
   if (key == 'f')
      return kTumbleDown;
   if (key == 'g')
      return kHalfSpeed;
   if (key == 'h')
      return kDoubleSpeed;
   if (key == 'j')
      return kDotted8th;
   if (key == 'k')
      return kQuarterTriplets;
   if (key == 'l')
      return kFree;
   return kNumStutterTypes;
}

void StutterControl::GetModuleDimensions(float& width, float& height)
{
   width = 175;
   height = 172;
}

StutterParams StutterControl::GetStutter(StutterControl::StutterType type)
{
   switch (type)
   {
      case kHalf:
         return StutterParams(kInterval_2n, 1);
      case kQuarter:
         return StutterParams(kInterval_4n, 1);
      case k8th:
         return StutterParams(kInterval_8n, 1);
      case k16th:
         return StutterParams(kInterval_16n, 1);
      case k32nd:
         return StutterParams(kInterval_32n, 1);
      case k64th:
         return StutterParams(kInterval_64n, 1);
      case kReverse:
         return StutterParams(kInterval_2n, -1);
      case kRampIn:
         return StutterParams(kInterval_8n, 0, 1, 500);
      case kRampOut:
         return StutterParams(kInterval_2n, 1, 0, 700);
      case kTumbleUp:
         return StutterParams(kInterval_8n, 1, 10, 10000);
      case kTumbleDown:
         return StutterParams(kInterval_32n, 1, 0, 2000);
      case kHalfSpeed:
         return StutterParams(kInterval_8n, .5f);
      case kDoubleSpeed:
         return StutterParams(kInterval_8n, 2);
      case kDotted8th:
         return StutterParams(kInterval_8nd, 1);
      case kQuarterTriplets:
         return StutterParams(kInterval_4nt, 1);
      case kFree:
         return StutterParams(kInterval_None, 1);
      case kNumStutterTypes:
         break;
   }
   assert(false);
   return StutterParams(kInterval_None, 1);
}

void StutterControl::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mStutterProcessor.SetEnabled(time, IsEnabled());

   for (int i = 0; i < kNumStutterTypes; ++i)
   {
      if (checkbox == mStutterCheckboxes[i])
      {
         SendStutter(time, GetStutter((StutterType)i), mStutter[i]);
      }
   }
}

void StutterControl::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void StutterControl::OnControllerPageSelected()
{
   UpdateGridLights();
}

void StutterControl::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int index = x + y * grid->NumCols();
   double time = NextBufferTime(false);
   if (index < kNumStutterTypes)
   {
      mStutter[index] = velocity > 0;
      SendStutter(time, GetStutter((StutterType)index), mStutter[index]);
   }
}

void StutterControl::PlayNote(NoteMessage note)
{
   int index = note.pitch % kNumStutterTypes;
   mStutter[index] = note.velocity > 0;
   SendStutter(note.time, GetStutter((StutterType)index), mStutter[index]);
}

void StutterControl::UpdateGridLights()
{
   if (mGridControlTarget == nullptr || mGridControlTarget->GetGridController() == nullptr)
      return;

   for (int i = 0; i < kNumStutterTypes; ++i)
   {
      mGridControlTarget->GetGridController()->SetLight(i % mGridControlTarget->GetGridController()->NumCols(), i / mGridControlTarget->GetGridController()->NumCols(), mStutter[i] ? kGridColor1Bright : kGridColor1Dim);
   }
}

bool StutterControl::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;

         if (y < 2)
         {
            int index = x + y * 8;
            mStutter[index] = midiValue > 0;
            SendStutter(gTime, GetStutter((StutterType)index), mStutter[index]);
         }

         return true;
      }
   }

   return false;
}

void StutterControl::UpdatePush2Leds(Push2Control* push2)
{
   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor;

         if (y < 2)
         {
            int index = x + y * 8;
            if (mStutter[index])
               pushColor = 2;
            else
               pushColor = 1;
         }
         else
         {
            pushColor = 0;
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }
}

void StutterControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void StutterControl::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
