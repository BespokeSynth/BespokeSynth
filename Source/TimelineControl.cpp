//
//  TimelineControl.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#include "TimelineControl.h"
#include "Transport.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

TimelineControl::TimelineControl()
: mTime(0)
, mTimeSlider(nullptr)
, mWidth(400)
, mLoop(false)
, mLoopCheckbox(nullptr)
, mNumMeasures(32)
, mLoopStart(0)
, mLoopEnd(8)
, mLoopStartSlider(nullptr)
, mLoopEndSlider(nullptr)
{
}

void TimelineControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTimeSlider = new FloatSlider(this,"measure",3,3,GetSliderWidth(),15,&mTime,0,mNumMeasures);
   mLoopCheckbox = new Checkbox(this,"loop",-1,-1,&mLoop);
   mLoopStartSlider = new IntSlider(this,"loop start",-1,-1,GetSliderWidth(),15,&mLoopStart,0,mNumMeasures);
   mLoopEndSlider = new IntSlider(this,"loop end",-1,-1,GetSliderWidth(),15,&mLoopEnd,0,mNumMeasures);
   
   mLoopCheckbox->PositionTo(mTimeSlider, kAnchor_Right);
   mLoopStartSlider->PositionTo(mTimeSlider, kAnchor_Below);
   mLoopEndSlider->PositionTo(mLoopStartSlider, kAnchor_Below);
   
   mLoopStartSlider->SetShowing(mLoop);
   mLoopEndSlider->SetShowing(mLoop);
}

TimelineControl::~TimelineControl()
{
}

void TimelineControl::DrawModule()
{
   mTime = TheTransport->GetMeasureTime(gTime);
   
   if (Minimized() || IsVisible() == false)
      return;
   
   mTimeSlider->Draw();
   mLoopCheckbox->Draw();
   mLoopStartSlider->Draw();
   mLoopEndSlider->Draw();
}

void TimelineControl::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mLoop ? 57 : 21;
}

void TimelineControl::Resize(float width, float height)
{
   mWidth = width;
   mTimeSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopStartSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopEndSlider->SetDimensions(GetSliderWidth(), 15);
   mLoopCheckbox->PositionTo(mTimeSlider, kAnchor_Right);
   mLoopStartSlider->PositionTo(mTimeSlider, kAnchor_Below);
   mLoopEndSlider->PositionTo(mLoopStartSlider, kAnchor_Below);
}

void TimelineControl::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mLoopCheckbox)
   {
      mLoopStartSlider->SetShowing(mLoop);
      mLoopEndSlider->SetShowing(mLoop);
      if (mLoop)
         TheTransport->SetLoop(mLoopStart, mLoopEnd);
      else
         TheTransport->ClearLoop();
   }
}

void TimelineControl::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mTimeSlider)
   {
      TheTransport->SetMeasure(int(mTime));
      TheTransport->SetMeasurePos(mTime - int(mTime));
   }
}

void TimelineControl::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLoopStartSlider || slider == mLoopEndSlider)
   {
      if (slider == mLoopStartSlider)
      {
         mLoopStart = MIN(mLoopStart, mNumMeasures-1);
         mLoopEnd = MAX(mLoopEnd, mLoopStart + 1);
      }
      if (slider == mLoopEndSlider)
      {
         mLoopEnd = MAX(mLoopEnd, 1);
         mLoopStart = MIN(mLoopStart, mLoopEnd - 1);
      }
      if (mLoop)
         TheTransport->SetLoop(mLoopStart, mLoopEnd);
   }
}

void TimelineControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadFloat("width", moduleInfo, 390, 100, 99999, K(isTextField));
   mModuleSaveData.LoadInt("num_measures", moduleInfo, 32, 1, 1024, K(isTextField));
   
   SetUpFromSaveData();
}

void TimelineControl::SetUpFromSaveData()
{
   Resize(mModuleSaveData.GetFloat("width"), 0);
   mNumMeasures = mModuleSaveData.GetInt("num_measures");
   mTimeSlider->SetExtents(0, mNumMeasures);
   mLoopStartSlider->SetExtents(0, mNumMeasures);
   mLoopEndSlider->SetExtents(0, mNumMeasures);
}

void TimelineControl::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["width"] = mWidth;
}
