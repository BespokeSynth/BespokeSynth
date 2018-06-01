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
, mGridController(nullptr)
{
   for (int i=0; i<kNumStutterTypes; ++i)
      mStutter[i] = false;
}

void StutterControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGridController = new GridController(this, 80, 40);
   
   int x = 4;
   int y = 40;
   mStutterCheckboxes[kHalf] = new Checkbox(this,"half note",x,y,&mStutter[kHalf]);
   y += 18;
   mStutterCheckboxes[kQuarter] = new Checkbox(this,"quarter",x,y,&mStutter[kQuarter]);
   y += 18;
   mStutterCheckboxes[k8th] = new Checkbox(this,"8th",x,y,&mStutter[k8th]);
   y += 18;
   mStutterCheckboxes[k16th] = new Checkbox(this,"16th",x,y,&mStutter[k16th]);
   y += 18;
   mStutterCheckboxes[k32nd] = new Checkbox(this,"32nd",x,y,&mStutter[k32nd]);
   y += 18;
   mStutterCheckboxes[k64th] = new Checkbox(this,"64th",x,y,&mStutter[k64th]);
   y += 18;
   mStutterCheckboxes[kReverse] = new Checkbox(this,"reverse",x,y,&mStutter[kReverse]);
   y += 18;
   mStutterCheckboxes[kRampIn] = new Checkbox(this,"ramp in",x,y,&mStutter[kRampIn]);
   y += 18;
   mStutterCheckboxes[kRampOut] = new Checkbox(this,"ramp out",x,y,&mStutter[kRampOut]);
   y += 18;
   mStutterCheckboxes[kTumbleUp] = new Checkbox(this,"tumble up",x,y,&mStutter[kTumbleUp]);
   y += 18;
   mStutterCheckboxes[kTumbleDown] = new Checkbox(this,"tumble down",x,y,&mStutter[kTumbleDown]);
   y += 18;
   mStutterCheckboxes[kHalfSpeed] = new Checkbox(this,"half speed",x,y,&mStutter[kHalfSpeed]);
   y += 18;
   mStutterCheckboxes[kDoubleSpeed] = new Checkbox(this,"double speed",x,y,&mStutter[kDoubleSpeed]);
   y += 18;
   mStutterCheckboxes[kQuarterTriplets] = new Checkbox(this,"triplets",x,y,&mStutter[kQuarterTriplets]);
   y += 18;
   mStutterCheckboxes[kDotted8th] = new Checkbox(this,"dotted eighth",x,y,&mStutter[kDotted8th]);
   y += 18;
   mStutterCheckboxes[kFree] = new Checkbox(this,"free",x,y,&mStutter[kFree]);
   y += 18;
   mFreeLengthSlider = new FloatSlider(this,"free length",x,y,102,15,&mStutterProcessor.mFreeStutterLength,.005f,.25f);
   y += 18;
   mFreeSpeedSlider = new FloatSlider(this,"free speed",x,y,102,15,&mStutterProcessor.mFreeStutterSpeed,0,2);
}

StutterControl::~StutterControl()
{
}

void StutterControl::Process(double time)
{
   Profiler profiler("StutterControl");
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   if (GetTarget())
   {
      mStutterProcessor.ProcessAudio(time, GetBuffer());
      
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
      }
   }
   
   GetBuffer()->Reset();
}

void StutterControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mStutterProcessor.DrawStutterBuffer(4, 3, 90, 35);
   
   for (int i=0; i<kNumStutterTypes; ++i)
      mStutterCheckboxes[i]->Draw();
   mFreeLengthSlider->Draw();
   mFreeSpeedSlider->Draw();
   mGridController->Draw();
}

void StutterControl::SendStutter(StutterParams stutter, bool on)
{
   if (on)
      mStutterProcessor.StartStutter(stutter);
   else
      mStutterProcessor.EndStutter(stutter);
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
      return kQuarterTriplets;
   if (key == 'k')
      return kDotted8th;
   if (key == 'l')
      return kFree;
   return kNumStutterTypes;
}

void StutterControl::GetModuleDimensions(int& x, int& y)
{
   x = 110;
   y = 80 + kNumStutterTypes*18;
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
      case kQuarterTriplets:
         return StutterParams(kInterval_4nt, 1);
      case kDotted8th:
         return StutterParams(kInterval_8nd, 1);
      case kFree:
         return StutterParams(kInterval_None, 1);
      case kNumStutterTypes:
         break;
   }
   assert(false);
   return StutterParams(kInterval_None, 1);
}

void StutterControl::CheckboxUpdated(Checkbox* checkbox)
{
   for (int i=0; i<kNumStutterTypes; ++i)
   {
      if (checkbox == mStutterCheckboxes[i])
      {
         SendStutter(GetStutter((StutterType)i), mStutter[i]);
      }
   }
}

void StutterControl::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void StutterControl::OnControllerPageSelected()
{
   UpdateGridLights();
}

void StutterControl::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int index = x + y * grid->NumCols();
   if (index < kNumStutterTypes)
   {
      mStutter[index] = velocity > 0;
      SendStutter(GetStutter((StutterType)index), mStutter[index]);
   }
}

void StutterControl::UpdateGridLights()
{
   if (mGridController == nullptr)
      return;
   
   for (int i=0; i<kNumStutterTypes; ++i)
   {
      mGridController->SetLight(i % mGridController->NumCols(), i / mGridController->NumCols(), mStutter[i] ? kGridColor1Bright : kGridColor1Dim);
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
