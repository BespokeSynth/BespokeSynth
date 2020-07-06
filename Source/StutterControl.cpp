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
   
   mGridController = new GridController(this, "grid", 5, 145);
   
   mStutterCheckboxes[kHalf] = new Checkbox(this,"half note",4,3,&mStutter[kHalf]);
   mStutterCheckboxes[kQuarter] = new Checkbox(this,"quarter",mStutterCheckboxes[kHalf],kAnchor_Below,&mStutter[kQuarter]);
   mStutterCheckboxes[k8th] = new Checkbox(this,"8th",mStutterCheckboxes[kQuarter],kAnchor_Below,&mStutter[k8th]);
   mStutterCheckboxes[k16th] = new Checkbox(this,"16th",mStutterCheckboxes[k8th],kAnchor_Below,&mStutter[k16th]);
   mStutterCheckboxes[k32nd] = new Checkbox(this,"32nd",mStutterCheckboxes[k16th],kAnchor_Below,&mStutter[k32nd]);
   mStutterCheckboxes[k64th] = new Checkbox(this,"64th",mStutterCheckboxes[k32nd],kAnchor_Below,&mStutter[k64th]);
   mStutterCheckboxes[kReverse] = new Checkbox(this,"reverse",mStutterCheckboxes[k64th],kAnchor_Below,&mStutter[kReverse]);
   mStutterCheckboxes[kRampIn] = new Checkbox(this,"ramp in",mStutterCheckboxes[kReverse],kAnchor_Below,&mStutter[kRampIn]);
   mStutterCheckboxes[kRampOut] = new Checkbox(this,"ramp out",mStutterCheckboxes[kHalf],kAnchor_Right,&mStutter[kRampOut]);
   mStutterCheckboxes[kTumbleUp] = new Checkbox(this,"tumble up",mStutterCheckboxes[kRampOut],kAnchor_Below,&mStutter[kTumbleUp]);
   mStutterCheckboxes[kTumbleDown] = new Checkbox(this,"tumble down",mStutterCheckboxes[kTumbleUp],kAnchor_Below,&mStutter[kTumbleDown]);
   mStutterCheckboxes[kHalfSpeed] = new Checkbox(this,"half speed",mStutterCheckboxes[kTumbleDown],kAnchor_Below,&mStutter[kHalfSpeed]);
   mStutterCheckboxes[kDoubleSpeed] = new Checkbox(this,"double speed",mStutterCheckboxes[kHalfSpeed],kAnchor_Below,&mStutter[kDoubleSpeed]);
   mStutterCheckboxes[kQuarterTriplets] = new Checkbox(this,"triplets",mStutterCheckboxes[kDoubleSpeed],kAnchor_Below,&mStutter[kQuarterTriplets]);
   mStutterCheckboxes[kDotted8th] = new Checkbox(this,"dotted eighth",mStutterCheckboxes[kQuarterTriplets],kAnchor_Below,&mStutter[kDotted8th]);
   mStutterCheckboxes[kFree] = new Checkbox(this,"free",mStutterCheckboxes[kDotted8th],kAnchor_Below,&mStutter[kFree]);
   mFreeLengthSlider = new FloatSlider(this,"free length",mStutterCheckboxes[kFree],kAnchor_Below,102,15,&mStutterProcessor.mFreeStutterLength,.005f,.25f);
   mFreeSpeedSlider = new FloatSlider(this,"free speed",mFreeLengthSlider,kAnchor_Below,102,15,&mStutterProcessor.mFreeStutterSpeed,0,2);
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

void StutterControl::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int index = pitch % kNumStutterTypes;
   mStutter[index] = velocity > 0;
   SendStutter(GetStutter((StutterType)index), mStutter[index]);
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
