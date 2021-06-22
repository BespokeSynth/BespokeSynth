//
//  SliderSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/25/14.
//
//

#include "SliderSequencer.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "DrumPlayer.h"

SliderSequencer::SliderSequencer()
: mLastMeasurePos(0)
, mDivision(1)
, mDivisionSlider(nullptr)
{
   
   for (int i=0; i<8; ++i)
      mSliderLines.push_back(new SliderLine(this,10,40+i*15,i));
}

void SliderSequencer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void SliderSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDivisionSlider = new IntSlider(this,"division",10,20,100,15,&mDivision,1,4);
   
   for (int i=0; i<mSliderLines.size(); ++i)
      mSliderLines[i]->CreateUIControls();
}

SliderSequencer::~SliderSequencer()
{
   TheTransport->RemoveAudioPoller(this);
   
   for (int i=0; i<mSliderLines.size(); ++i)
      delete mSliderLines[i];
}

float SliderSequencer::MeasurePos(double time)
{
   float pos = TheTransport->GetMeasurePos(time) * mDivision;
   while (pos > 1)
      pos -= 1;
   
   return pos;
}

void SliderSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(SliderSequencer);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   float current = MeasurePos(gTime + gBufferSize);
   
   for (int i=0; i<mSliderLines.size(); ++i)
   {
      if (mSliderLines[i]->mVelocity == 0)
         continue;
      
      if ((mSliderLines[i]->mPoint > mLastMeasurePos || mLastMeasurePos > current) && mSliderLines[i]->mPoint <= current)
      {
         double remainder = current - mSliderLines[i]->mPoint;
         FloatWrap(remainder, 1);
         double remainderMs = TheTransport->MsPerBar() * remainder;
         double time = gTime + gBufferSize - remainderMs;
         PlayNoteOutput(time, mSliderLines[i]->mPitch, mSliderLines[i]->mVelocity * 127, -1);
         PlayNoteOutput(time + TheTransport->GetDuration(kInterval_16n), mSliderLines[i]->mPitch, 0, -1);
         mSliderLines[i]->mPlayTime = gTime;
      }
      
      mSliderLines[i]->mPlaying = mSliderLines[i]->mPlayTime + 100 > gTime;
   }
   
   mLastMeasurePos = current;
}

void SliderSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mDivisionSlider->Draw();
   for (int i=0; i<mSliderLines.size(); ++i)
      mSliderLines[i]->Draw();
   
   if (mEnabled)
   {
      ofPushStyle();
      ofSetLineWidth(1);
      ofSetColor(0,255,0);
      ofFill();
      ofRect(10+180*MeasurePos(gTime), 40, 1, 120);
      ofPopStyle();
   }
}

void SliderSequencer::CheckboxUpdated(Checkbox* checkbox)
{
}

void SliderSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SliderSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void SliderSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void SliderSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SliderSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


SliderLine::SliderLine(SliderSequencer* owner, int x, int y, int index)
: mSlider(nullptr)
, mPoint(0)
, mVelocity(0)
, mVelocitySlider(nullptr)
, mPitch(0)
, mNoteSelector(nullptr)
, mPlayTime(0)
, mPlaying(false)
, mPlayingCheckbox(nullptr)
, mX(x)
, mY(y)
, mOwner(owner)
, mIndex(index)
{
}

void SliderLine::CreateUIControls()
{
   mSlider = new FloatSlider(mOwner,("time"+ofToString(mIndex)).c_str(),mX,mY,180,15,&mPoint,0,1);
   mVelocitySlider = new FloatSlider(mOwner,("vel"+ofToString(mIndex)).c_str(),mX+185,mY,80,15,&mVelocity,0,.99f,2);
   mNoteSelector = new TextEntry(mOwner,("note"+ofToString(mIndex)).c_str(),mX+270,mY,4,&mPitch,0,127);
   mPlayingCheckbox = new Checkbox(mOwner,("playing"+ofToString(mIndex)).c_str(),HIDDEN_UICONTROL,HIDDEN_UICONTROL,&mPlaying);
}

void SliderLine::Draw()
{
   ofPushStyle();
   ofFill();
   ofSetColor(255,255,0);
   if (mPlaying)
      ofRect(mX,mY,10,10);
   ofPopStyle();
   
   mSlider->Draw();
   mNoteSelector->Draw();
   mVelocitySlider->Draw();
}


