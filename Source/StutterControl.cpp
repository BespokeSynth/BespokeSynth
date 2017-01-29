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

StutterControl::StutterControl()
: mFreeLength(.1f)
, mFreeSpeed(1)
{
   for (int i=0; i<kNumStutterTypes; ++i)
      mStutter[i] = false;
}

void StutterControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   int x = 4;
   int y = 4;
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
   mFreeLengthSlider = new FloatSlider(this,"free length",x,y,100,15,&mFreeLength,.005f,.25f);
   y += 18;
   mFreeSpeedSlider = new FloatSlider(this,"free speed",x,y,100,15,&mFreeSpeed,0,2);
}

StutterControl::~StutterControl()
{
}

void StutterControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (int i=0; i<kNumStutterTypes; ++i)
      mStutterCheckboxes[i]->Draw();
   mFreeLengthSlider->Draw();
   mFreeSpeedSlider->Draw();
}

void StutterControl::SendStutter(StutterParams stutter, bool on)
{
   if (on)
   {
      for (auto listener : mListeners)
         listener->StartStutter(stutter);
   }
   else
   {
      for (auto listener : mListeners)
         listener->EndStutter(stutter);
   }
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

void StutterControl::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   if (GetKeyModifiers() == kModifier_Shift)
   {
      StutterType type = GetStutterFromKey(key);
      if (type != kNumStutterTypes)
      {
         mStutter[type] = true;
         SendStutter(GetStutter(type), true);
      }
   }
}

void StutterControl::KeyReleased(int key)
{
   StutterType type = GetStutterFromKey(key);
   if (type != kNumStutterTypes)
   {
      mStutter[type] = false;
      SendStutter(GetStutter(type), false);
   }
}

void StutterControl::GetModuleDimensions(int& x, int& y)
{
   x= 75;
   y= 44 + kNumStutterTypes*18;
}

void StutterControl::AddListener(Stutter* stutter)
{
   mListeners.push_back(stutter);
}

void StutterControl::RemoveListener(Stutter* stutter)
{
   mListeners.remove(stutter);
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
