//
//  CircleSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/3/15.
//
//

#include "CircleSequencer.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "DrumPlayer.h"

namespace
{
   ofVec2f PolToCar(float pos, float radius)
   {
      return ofVec2f(radius*sin(pos*TWO_PI),radius*-cos(pos*TWO_PI));
   }
   
   ofVec2f CarToPol(float x, float y)
   {
      float pos = atan2(x,-y) / TWO_PI;
      FloatWrap(pos, 1);
      return ofVec2f(pos, sqrtf(x*x+y*y));
   }
}

CircleSequencer::CircleSequencer()
{
   TheTransport->AddAudioPoller(this);
   
   for (int i=0; i<4; ++i)
      mCircleSequencerRings.push_back(new CircleSequencerRing(this,i));
}

void CircleSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->CreateUIControls();
}

CircleSequencer::~CircleSequencer()
{
   TheTransport->RemoveAudioPoller(this);
   
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      delete mCircleSequencerRings[i];
}

void CircleSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(CircleSequencer);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->OnTransportAdvanced(amount);
}

void CircleSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->Draw();
   
   ofPushStyle();
   ofSetColor(ofColor::lime);
   float pos = TheTransport->GetMeasurePos(gTime);
   ofVec2f end = PolToCar(pos,100);
   ofLine(100,100,100+end.x,100+end.y);
   ofPopStyle();
}

void CircleSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->OnClicked(x,y,right);
}

void CircleSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->MouseReleased();
}

bool CircleSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   for (int i=0; i<mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->MouseMoved(x,y);
   return false;
}

void CircleSequencer::CheckboxUpdated(Checkbox* checkbox)
{
}

void CircleSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void CircleSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void CircleSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void CircleSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


CircleSequencerRing::CircleSequencerRing(CircleSequencer* owner, int index)
: mLength(4)
, mLengthSelector(nullptr)
, mNote(index)
, mNoteSelector(nullptr)
, mOwner(owner)
, mIndex(index)
, mOffset(0)
, mOffsetSlider(nullptr)
, mCurrentlyClickedStepIdx(-1)
, mHighlightStepIdx(-1)
{
   bzero(mSteps,sizeof(float)*CIRCLE_SEQUENCER_MAX_STEPS);
}

void CircleSequencerRing::CreateUIControls()
{
   int y = mIndex*20+20;
   mLengthSelector = new DropdownList(mOwner,("length"+ofToString(mIndex)).c_str(),220,y,&mLength);
   mNoteSelector = new TextEntry(mOwner,("note"+ofToString(mIndex)).c_str(),260,y,4,&mNote,0,127);
   mOffsetSlider = new FloatSlider(mOwner,("offset"+ofToString(mIndex)).c_str(),300,y,90,15,&mOffset,-.25f,.25f,2);
   
   for (int i=0; i<CIRCLE_SEQUENCER_MAX_STEPS; ++i)
      mLengthSelector->AddLabel(ofToString(i+1).c_str(), i+1);
}

void CircleSequencerRing::Draw()
{
   ofPushStyle();
   ofSetCircleResolution(40);
   ofNoFill();
   ofCircle(100,100,GetRadius());
   ofFill();
   for (int i=0; i<mLength; ++i)
   {
      float pos = float(i)/mLength + mOffset;
      ofVec2f p1 = PolToCar(pos, GetRadius()-3);
      ofVec2f p2 = PolToCar(pos, GetRadius()+3);
      ofLine(p1.x+100,p1.y+100,p2.x+100,p2.y+100);
      ofVec2f point = PolToCar(pos, GetRadius());
      
      if (mSteps[i] > 0)
         ofCircle(100+point.x,100+point.y,3+6*mSteps[i]);
      
      if (i == mHighlightStepIdx)
      {
         ofPushStyle();
         ofSetColor(255,255,255,100);
         ofSetLineWidth(.5f);
         ofNoFill();
         ofCircle(100+point.x,100+point.y,3+6);
         ofPopStyle();
      }
   }
   ofPopStyle();
   mLengthSelector->Draw();
   mNoteSelector->Draw();
   mOffsetSlider->Draw();
}

int CircleSequencerRing::GetStepIndex(int x, int y, float &radiusOut)
{
   ofVec2f polar = CarToPol(x-100,y-100);
   float pos = polar.x - mOffset;
   FloatWrap(pos,1);
   int idx = int(pos * mLength + .5f) % mLength;
   
   ofVec2f stepPos = PolToCar(float(idx)/mLength + mOffset, GetRadius());
   if (ofDistSquared(x, y, stepPos.x+100, stepPos.y+100) < 7*7)
   {
      radiusOut = polar.y;
      return idx;
   }
   
   return -1;
}

void CircleSequencerRing::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mCurrentlyClickedStepIdx = GetStepIndex(x, y, mLastMouseRadius);
   if (mCurrentlyClickedStepIdx != -1)
   {
      if (mSteps[mCurrentlyClickedStepIdx])
         mSteps[mCurrentlyClickedStepIdx] = 0;
      else
         mSteps[mCurrentlyClickedStepIdx] = .5f;
   }
}

void CircleSequencerRing::MouseReleased()
{
   mCurrentlyClickedStepIdx = -1;
}

void CircleSequencerRing::MouseMoved(float x, float y)
{
   if (mCurrentlyClickedStepIdx != -1)
   {
      ofVec2f polar = CarToPol(x-100,y-100);
      float change = (polar.y - mLastMouseRadius) / 50.0f;
      
      mSteps[mCurrentlyClickedStepIdx] = ofClamp(mSteps[mCurrentlyClickedStepIdx]+change,0,1);
      
      mLastMouseRadius = polar.y;
   }
   else
   {
      float radius;
      mHighlightStepIdx = GetStepIndex(x, y, radius);
   }
}

void CircleSequencerRing::OnTransportAdvanced(float amount)
{
   PROFILER(CircleSequencerRing);
   
   float pos = TheTransport->GetMeasurePos(gTime) - mOffset;
   FloatWrap(pos,1);
   int oldQuantized;
   if (amount > pos)
      oldQuantized = -1;
   else
      oldQuantized = int((pos-amount) * mLength);
   int quantized = int(pos * mLength);
   
   if (quantized != oldQuantized && mSteps[quantized] > 0)
   {
      mOwner->PlayNoteOutput(gTime, mNote, mSteps[quantized] * 127, -1);
   }
}
