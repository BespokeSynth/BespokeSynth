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
//  EuclideanSequencer.cpp
//  Bespoke
//
//  Created by Jack van Klaren on Mar 17 2024.
//  Based on CircleSequencer by Ryan Challinor
//
//

#include "EuclideanSequencer.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "DrumPlayer.h"
#include "PatchCableSource.h"

#define _X86_ // required for debugapi
#include "debugapi.h"

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

EuclideanSequencer::EuclideanSequencer()
{

   for (int i = 0; i < 4; ++i)
      mEuclideanSequencerRings.push_back(new EuclideanSequencerRing(this, i));
}

void EuclideanSequencer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void EuclideanSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->CreateUIControls();
}

EuclideanSequencer::~EuclideanSequencer()
{
   TheTransport->RemoveAudioPoller(this);

   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      delete mEuclideanSequencerRings[i];
}

void EuclideanSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(EuclideanSequencer);

   if (!mEnabled)
      return;

   ComputeSliders(0);

   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->OnTransportAdvanced(amount);
}

void EuclideanSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->Draw();

   ofPushStyle();
   ofSetColor(ofColor::lime);
   float pos = TheTransport->GetMeasurePos(gTime);
   ofVec2f end = PolToCar(pos, 100);
   ofLine(100, 100, 100 + end.x, 100 + end.y);
   ofPopStyle();
}

void EuclideanSequencer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->OnClicked(x, y, right);
}

void EuclideanSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->MouseReleased();
}

bool EuclideanSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->MouseMoved(x, y);
   return false;
}

void EuclideanSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void EuclideanSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void EuclideanSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->IntSliderUpdated(slider, oldVal, time);
}

void EuclideanSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void EuclideanSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void EuclideanSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void EuclideanSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mEuclideanSequencerRings.size();
   for (size_t i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->SaveState(out);
}

void EuclideanSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int numRings;
   in >> numRings;
   for (size_t i = 0; i < mEuclideanSequencerRings.size() && i < numRings; ++i)
      mEuclideanSequencerRings[i]->LoadState(in);
}


EuclideanSequencerRing::EuclideanSequencerRing(EuclideanSequencer* owner, int index)
: mPitch(index)
, mOwner(owner)
, mIndex(index)
{
   mSteps.fill(0);
}

void EuclideanSequencerRing::CreateUIControls()
{
   int y = mIndex * 20 + 20;

   switch (mIndex)
   {
      case 0:
         mLength = 4;
         mOnset = 4;
         mRotation = 0;
         break;
      case 1:
         mLength = 12;
         mOnset = 2;
         mRotation = 3;
         break;
      case 2:
         mLength = 8;
         mOnset = 4;
         mRotation = 1;
         break;
      case 3:
         mLength = 10;
         mOnset = 2;
         mRotation = 2;
         break;
   }

   mLengthSlider = new IntSlider(mOwner, ("steps" + ofToString(mIndex)).c_str(), 220, y, 90, 15, &mLength, 0, EUCLIDEAN_SEQUENCER_MAX_STEPS);
   mOnsetSlider = new IntSlider(mOwner, ("onsets" + ofToString(mIndex)).c_str(), 315, y, 90, 15, &mOnset, 0, EUCLIDEAN_SEQUENCER_MAX_STEPS);
   mRotationSlider = new IntSlider(mOwner, ("rotation" + ofToString(mIndex)).c_str(), 410, y, 90, 15, &mRotation, -8, 8);
   mOffsetSlider = new FloatSlider(mOwner, ("offset" + ofToString(mIndex)).c_str(), 505, y, 90, 15, &mOffset, -.25f, .25f, 2);
   mNoteSelector = new TextEntry(mOwner, ("note" + ofToString(mIndex)).c_str(), 600, y, 4, &mPitch, 0, 127);

   mDestinationCable = new AdditionalNoteCable();
   mDestinationCable->SetPatchCableSource(new PatchCableSource(mOwner, kConnectionType_Note));
   mDestinationCable->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   mOwner->AddPatchCableSource(mDestinationCable->GetPatchCableSource());
   mDestinationCable->GetPatchCableSource()->SetManualPosition(648, y + 7);

   // Calculate Euclidean steps
   IntSliderUpdated(mLengthSlider, 0, 0);
}

void EuclideanSequencerRing::Draw()
{
   ofPushStyle();

   switch (mIndex)
   {
      case 0:
         if (mLength == 0 || mOnset == 0)
         {
            ofSetColor(150, 100, 100);
         }
         else
         {
            ofSetColor(255, 150, 150);
         }
         break;
      case 1:
         if (mLength == 0 || mOnset == 0)
         {
            ofSetColor(150, 150, 100);
         }
         else
         {
            ofSetColor(255, 255, 150);
         }
         break;
      case 2:
         if (mLength == 0 || mOnset == 0)
         {
            ofSetColor(100, 150, 150);
         }
         else
         {
            ofSetColor(150, 255, 255);
         }
         break;
      case 3:
         if (mLength == 0 || mOnset == 0)
         {
            ofSetColor(100, 100, 150);
         }
         else
         {
            ofSetColor(150, 150, 255);
         }
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
   mLengthSlider->Draw();
   mOnsetSlider->Draw();
   mRotationSlider->Draw();
   mOffsetSlider->Draw();
   mNoteSelector->Draw();
}

int EuclideanSequencerRing::GetStepIndex(int x, int y, float& radiusOut)
{
   if (mLength == 0)
   {
      return -1;
   }
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

void EuclideanSequencerRing::OnClicked(float x, float y, bool right)
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

void EuclideanSequencerRing::MouseReleased()
{
   mCurrentlyClickedStepIdx = -1;
}

void EuclideanSequencerRing::MouseMoved(float x, float y)
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

void EuclideanSequencerRing::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{

   if (slider == mLengthSlider || slider == mOnsetSlider)
   {
      //      char s[256];
      //      sprintf(s, "mIndex: %d Slider: %s oldVal: %d\n", mIndex, slider->Name(), oldVal);
      //      OutputDebugStringA(s);
      //      sprintf(s, "mLengthSlider.GetValue: %f mOnsetSlider.GetValue: %f\n", mLengthSlider->GetValue(), mOnsetSlider->GetValue());
      //      OutputDebugStringA(s);

      mLength = static_cast<int>(mLengthSlider->GetValue());
      mOnset = static_cast<int>(mOnsetSlider->GetValue());
      mRotation = static_cast<int>(mRotationSlider->GetValue());

      // Clear all steps
      mSteps.fill(0);

      // Nothing to do, return empty mSteps
      if (mLength == 0 || mOnset == 0)
      {
         return;
      }

      // Get Euclidean Rhythm, returns a string of 1's and 0's
      std::string sEuclid = GetEuclideanRhythm(mOnset, mLength, mRotation);
      //      OutputDebugStringA(sEuclid.c_str());

      // Fill mSteps
      for (int i = 0; i < mLength; i++)
      {
         if (sEuclid[i] == '1')
         {
            mSteps[i] = .5f;
         }
         else
         {
            mSteps[i] = 0;
         }
      }
   }

   if (slider == mRotationSlider)
   {
      // Do not generate a new GetEuclideanRhythm, but rotate mSteps
      // This way, manually modified onsets will remain

      std::array<float, EUCLIDEAN_SEQUENCER_MAX_STEPS> mTempSteps{};
      int newVal = static_cast<int>(mRotationSlider->GetValue());
      int rotOffset = (newVal - oldVal) % mLength;

      //      char s[256];
      //      sprintf(s, "mIndex: %d Slider: %s oldVal: %d newVal: %d rotOffset: %d\n", mIndex, slider->Name(), oldVal, newVal, rotOffset);
      //      OutputDebugStringA(s);

      // Save current mSteps
      mTempSteps = mSteps;

      // Fill mSteps with old data using rotOffset
      for (int i = 0; i < mLength; i++)
      {
         //         sprintf(s, "i: %d rotOffset: %d mLength: %d val: %d\n", i, rotOffset, mLength, (i + rotOffset + mLength) % mLength);
         //         OutputDebugStringA(s);
         mSteps[i] = mTempSteps[(i + rotOffset + mLength) % mLength]; // + mLength to avoid negative mod results
      }
   }
}


void EuclideanSequencerRing::OnTransportAdvanced(float amount)
{
   PROFILER(EuclideanSequencerRing);

   TransportListenerInfo info(nullptr, kInterval_CustomDivisor, OffsetInfo(mOffset, false), false);
   info.mCustomDivisor = mLength;

   double remainderMs;
   const int oldStep = TheTransport->GetQuantized(NextBufferTime(true) - gBufferSizeMs, &info);
   const int newStep = TheTransport->GetQuantized(NextBufferTime(true), &info, &remainderMs);
   const int oldMeasure = TheTransport->GetMeasure(NextBufferTime(true) - gBufferSizeMs);
   const int newMeasure = TheTransport->GetMeasure(NextBufferTime(true));

   if ((oldMeasure != newMeasure || oldStep != newStep) && mSteps[newStep] > 0)
   {
      const double time = NextBufferTime(true) - remainderMs;
      mOwner->PlayNoteOutput(time, mPitch, mSteps[newStep] * 127, -1);
      mOwner->PlayNoteOutput(time + 32.0 / mLength * TheTransport->GetDuration(kInterval_32n), mPitch, 0, -1);

      mDestinationCable->PlayNoteOutput(time, mPitch, mSteps[newStep] * 127, -1);
      mDestinationCable->PlayNoteOutput(time + 32.0 / mLength * TheTransport->GetDuration(kInterval_32n), mPitch, 0, -1);
   }
}

void EuclideanSequencerRing::SaveState(FileStreamOut& out)
{
   out << (int)mSteps.size();
   for (size_t i = 0; i < mSteps.size(); ++i)
      out << mSteps[i];
}

void EuclideanSequencerRing::LoadState(FileStreamIn& in)
{
   int numSteps;
   in >> numSteps;
   for (size_t i = 0; i < mSteps.size() && i < numSteps; ++i)
      in >> mSteps[i];
}

std::string EuclideanSequencerRing::GetEuclideanRhythm(int pulses, int steps, int rotation)
{
   std::vector<char> rhythm(steps, '0'); // Vector to store the rhythm
   int bucket = 0; // count steps until the next pulse
   bool hasPulse = false; // check if vector has a pulse

   // return rhythm filled with '0'
   if (pulses == 0)
   {
      return std::string(rhythm.begin(), rhythm.end());
   }

   // Fill rhythm with steps
   for (int i = 0; i < steps; i++)
   {
      bucket += pulses;
      if (bucket >= steps)
      {
         bucket -= steps;
         rhythm[i] = '1';
         hasPulse = true;
      }
   }

   // Rotate until pulse on first step
   while (hasPulse && rhythm[0] != '1')
   {
      std::rotate(rhythm.begin(), rhythm.begin() + 1, rhythm.end());
   }

   // Rotate the rhythm according to rotation parameter
   // Rotate 1 char at a time, to avoid overflows
   if (rotation >= 0)
   {
      for (int i = 0; i < rotation; i++)
      {
         std::rotate(rhythm.begin(), rhythm.begin() + 1, rhythm.end());
      }
   }
   else
   {
      for (int i = 0; i < -rotation; i++)
      {
         std::rotate(rhythm.rbegin(), rhythm.rbegin() + 1, rhythm.rend());
      }
   }

   return std::string(rhythm.begin(), rhythm.end());
}
