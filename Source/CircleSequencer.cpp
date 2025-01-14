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
      return ofVec2f(radius * sin(pos * TWO_PI), radius * -cos(pos * TWO_PI));
   }

   ofVec2f CarToPol(float x, float y)
   {
      float pos = FloatWrap(atan2(x, -y) / TWO_PI, 1);
      return ofVec2f(pos, sqrtf(x * x + y * y));
   }
}

CircleSequencer::CircleSequencer()
{

   for (int i = 0; i < 4; ++i)
      mCircleSequencerRings.push_back(new CircleSequencerRing(this, i));
}

void CircleSequencer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void CircleSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->CreateUIControls();
}

CircleSequencer::~CircleSequencer()
{
   TheTransport->RemoveAudioPoller(this);

   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      delete mCircleSequencerRings[i];
}

void CircleSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(CircleSequencer);

   if (!mEnabled)
      return;

   ComputeSliders(0);

   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->OnTransportAdvanced(amount);
}

void CircleSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->Draw();

   ofPushStyle();
   ofSetColor(ofColor::lime);
   float pos = TheTransport->GetMeasurePos(gTime);
   ofVec2f end = PolToCar(pos, 100);
   ofLine(100, 100, 100 + end.x, 100 + end.y);
   ofPopStyle();
}

void CircleSequencer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->OnClicked(x, y, right);
}

void CircleSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->MouseReleased();
}

bool CircleSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   for (int i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->MouseMoved(x, y);
   return false;
}

void CircleSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void CircleSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void CircleSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
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

void CircleSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mCircleSequencerRings.size();
   for (size_t i = 0; i < mCircleSequencerRings.size(); ++i)
      mCircleSequencerRings[i]->SaveState(out);
}

void CircleSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int numRings;
   in >> numRings;
   for (size_t i = 0; i < mCircleSequencerRings.size() && i < numRings; ++i)
      mCircleSequencerRings[i]->LoadState(in);
}


CircleSequencerRing::CircleSequencerRing(CircleSequencer* owner, int index)
: mPitch(index)
, mOwner(owner)
, mIndex(index)
{
   mSteps.fill(0);
}

void CircleSequencerRing::CreateUIControls()
{
   int y = mIndex * 20 + 20;
   mLengthSelector = new DropdownList(mOwner, ("length" + ofToString(mIndex)).c_str(), 220, y, &mLength);
   mNoteSelector = new TextEntry(mOwner, ("note" + ofToString(mIndex)).c_str(), 260, y, 4, &mPitch, 0, 127);
   mOffsetSlider = new FloatSlider(mOwner, ("offset" + ofToString(mIndex)).c_str(), 300, y, 90, 15, &mOffset, -.25f, .25f, 2);

   for (int i = 0; i < CIRCLE_SEQUENCER_MAX_STEPS; ++i)
      mLengthSelector->AddLabel(ofToString(i + 1).c_str(), i + 1);
}

void CircleSequencerRing::Draw()
{
   ofPushStyle();

   switch (mIndex)
   {
      case 0:
         ofSetColor(255, 150, 150);
         break;
      case 1:
         ofSetColor(255, 255, 150);
         break;
      case 2:
         ofSetColor(150, 255, 255);
         break;
      case 3:
         ofSetColor(150, 150, 255);
         break;
      default:
         ofSetColor(255, 255, 255);
         break;
   }

   ofSetCircleResolution(40);
   ofNoFill();
   ofCircle(100, 100, GetRadius());
   ofFill();
   for (int i = 0; i < mLength; ++i)
   {
      float pos = float(i) / mLength - mOffset;
      ofVec2f p1 = PolToCar(pos, GetRadius() - 3);
      ofVec2f p2 = PolToCar(pos, GetRadius() + 3);
      ofLine(p1.x + 100, p1.y + 100, p2.x + 100, p2.y + 100);
      ofVec2f point = PolToCar(pos, GetRadius());

      if (mSteps[i] > 0)
         ofCircle(100 + point.x, 100 + point.y, 3 + 6 * mSteps[i]);

      if (i == mHighlightStepIdx)
      {
         ofPushStyle();
         ofSetColor(255, 255, 255, 100);
         ofSetLineWidth(.5f);
         ofNoFill();
         ofCircle(100 + point.x, 100 + point.y, 3 + 6);
         ofPopStyle();
      }
   }
   ofPopStyle();
   mLengthSelector->Draw();
   mNoteSelector->Draw();
   mOffsetSlider->Draw();
}

int CircleSequencerRing::GetStepIndex(int x, int y, float& radiusOut)
{
   ofVec2f polar = CarToPol(x - 100, y - 100);
   float pos = FloatWrap(polar.x + mOffset, 1);
   int idx = int(pos * mLength + .5f) % mLength;

   ofVec2f stepPos = PolToCar(float(idx) / mLength - mOffset, GetRadius());
   if (ofDistSquared(x, y, stepPos.x + 100, stepPos.y + 100) < 7 * 7)
   {
      radiusOut = polar.y;
      return idx;
   }

   return -1;
}

void CircleSequencerRing::OnClicked(float x, float y, bool right)
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
      ofVec2f polar = CarToPol(x - 100, y - 100);
      float change = (polar.y - mLastMouseRadius) / 50.0f;

      mSteps[mCurrentlyClickedStepIdx] = ofClamp(mSteps[mCurrentlyClickedStepIdx] + change, 0, 1);

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

   TransportListenerInfo info(nullptr, kInterval_CustomDivisor, OffsetInfo(mOffset, false), false);
   info.mCustomDivisor = mLength + (mLength == 1); // +1 if mLength(Steps) == 1: fixes not playing onset 1 when mLength = 1: force oldStep <> newStep

   double remainderMs;
   const int oldStep = TheTransport->GetQuantized(NextBufferTime(true) - gBufferSizeMs, &info);
   const int newStep = TheTransport->GetQuantized(NextBufferTime(true), &info, &remainderMs);
   const int oldMeasure = TheTransport->GetMeasure(NextBufferTime(true) - gBufferSizeMs);
   const int newMeasure = TheTransport->GetMeasure(NextBufferTime(true));

   if (oldStep != newStep && mSteps[newStep] > 0)
   {
      const double time = NextBufferTime(true) - remainderMs;
      mOwner->PlayNoteOutput(NoteMessage(time, mPitch, mSteps[newStep] * 127));
      NoteInterval interval = kInterval_16n;
      if (mLength > 10 && mLength < 24)
         interval = kInterval_32n;
      else if (mLength >= 24)
         interval = kInterval_64n;
      mOwner->PlayNoteOutput(NoteMessage(time + TheTransport->GetDuration(interval), mPitch, 0));
   }
}

void CircleSequencerRing::SaveState(FileStreamOut& out)
{
   out << (int)mSteps.size();
   for (size_t i = 0; i < mSteps.size(); ++i)
      out << mSteps[i];
}

void CircleSequencerRing::LoadState(FileStreamIn& in)
{
   int numSteps;
   in >> numSteps;
   for (size_t i = 0; i < mSteps.size() && i < numSteps; ++i)
      in >> mSteps[i];
}
