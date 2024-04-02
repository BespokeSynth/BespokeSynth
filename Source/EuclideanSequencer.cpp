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

   int x = 210;
   int y = 5;
   int xcolumn = 115;
   mRndLengthChanceSlider = new FloatSlider(this, "step chance", 0 * xcolumn + x, y, 110, 15, &mRndLengthChance, 0.00f, 1.00f, 2);
   mRndLengthMaxSlider = new FloatSlider(this, "step max", 0 * xcolumn + x, y + 20, 110, 15, &mRndLengthMax, 0, 32, 0);
   mRndOnsetChanceSlider = new FloatSlider(this, "onset chance", 1 * xcolumn + x, y, 110, 15, &mRndOnsetChance, 0.00f, 1.00f, 2);
   mRndOnsetMaxSlider = new FloatSlider(this, "onset max", 1 * xcolumn + x, y + 20, 110, 15, &mRndOnsetMax, 0, 32, 0);
   mRndRotationChanceSlider = new FloatSlider(this, "rot chance", 2 * xcolumn + x, y, 110, 15, &mRndRotationChance, 0.00f, 1.00f, 2);
   mRndRotationMaxSlider = new FloatSlider(this, "rot max", 2 * xcolumn + x, y + 20, 110, 15, &mRndRotationMax, 0, 32, 0);
   mRndOffsetChanceSlider = new FloatSlider(this, "offs chance", 3 * xcolumn + x, y, 110, 15, &mRndOffsetChance, 0.00f, 1.00f, 2);
   mRndOffsetMaxSlider = new FloatSlider(this, "offs max", 3 * xcolumn + x, y + 20, 110, 15, &mRndOffsetMax, 0.00f, 0.25f, 2);
   mRndNoteChanceSlider = new FloatSlider(this, "note chance", 4 * xcolumn + x, y, 110, 15, &mRndNoteChance, 0.00f, 1.00f, 2);
   mRndOctaveLoSlider = new FloatSlider(this, "oct lo", 4 * xcolumn + x, y + 20, 55, 15, &mRndOctaveLo, 0.00f, 5.00f, 0);
   mRndOctaveHiSlider = new FloatSlider(this, "oct hi", 4 * xcolumn + x + 55, y + 20, 55, 15, &mRndOctaveHi, 0.00f, 5.00f, 0);
   

   x = 210;
   y = 65;
   xcolumn = 95;
   mRnd0Button = new ClickButton(this, "random0", 0 * xcolumn + x, y);
   mRnd1Button = new ClickButton(this, "random1", 1 * xcolumn + x, y);
   mRnd2Button = new ClickButton(this, "random2", 2 * xcolumn + x, y);
   mRnd3Button = new ClickButton(this, "random3", 3 * xcolumn + x, y);

   x = 595;
   mRandomizeButton = new ClickButton(this, "random", x, y);
   y = 45;
   mClearButton = new ClickButton(this, "clear", x, y);
   y = 90;
   mRndLengthButton = new ClickButton(this, "steps", x, y);
   mRndOnsetsButton = new ClickButton(this, "onsets", x, y + 20);
   mRndRotationButton = new ClickButton(this, "rotation", x, y + 40);
   mRndOffsetButton = new ClickButton(this, "offset", x, y + 60);
   mRndNoteButton = new ClickButton(this, "note", x, y + 80);

   int aPreset = (int)ofRandom(0, 1.9); // select a random default preset
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
   {
      mEuclideanSequencerRings[i]->CreateUIControls();
      mEuclideanSequencerRings[i]->InitPreset(aPreset);
   }
}

EuclideanSequencer::~EuclideanSequencer()
{
   TheTransport->RemoveAudioPoller(this);

   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      delete mEuclideanSequencerRings[i];
}

void EuclideanSequencer::Resize(float w, float h)
{
   mWidth = MAX(w, mWidthMin); // limit minimum width
   mWidth = MIN(mWidth, mWidthMax); // limit maximum width
   // mHeight = 200; // fixed height
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

   mRandomizeButton->Draw();
   mRndLengthButton->Draw();
   mRndOnsetsButton->Draw();
   mRndRotationButton->Draw();
   mRndOffsetButton->Draw();
   mRndNoteButton->Draw();
   mRnd0Button->Draw();
   mRnd1Button->Draw();
   mRnd2Button->Draw();
   mRnd3Button->Draw();
   mClearButton->Draw();

   mRndLengthChanceSlider->Draw();
   mRndLengthMaxSlider->Draw();
   mRndOnsetChanceSlider->Draw();
   mRndOnsetMaxSlider->Draw();
   mRndRotationChanceSlider->Draw();
   mRndRotationMaxSlider->Draw();
   mRndOffsetChanceSlider->Draw();
   mRndOffsetMaxSlider->Draw();
   mRndNoteChanceSlider->Draw();
   mRndOctaveLoSlider->Draw();
   mRndOctaveHiSlider->Draw();

   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->Draw();

   // Grey lines around circle sliders
   ofPushStyle();
   ofSetColor(128, 128, 128);
   ofSetLineWidth(.1f);
   ofLine(210, 85, 590, 85);
   ofLine(590, 85, 590, 185);
   ofPopStyle();
   
   // Rotating tranposrt line
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
   // Check if Lo > Hi or Hi < Lo
   if (slider == mRndOctaveLoSlider)
      if (mRndOctaveLo > mRndOctaveHi)
         mRndOctaveLo = mRndOctaveHi;
   {
   }
   if (slider == mRndOctaveHiSlider)
   {
      if (mRndOctaveHi < mRndOctaveLo)
         mRndOctaveHi = mRndOctaveLo;
   }
 
   // Handle slider updates of all ring sliders
   for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
      mEuclideanSequencerRings[i]->FloatSliderUpdated(slider, oldVal, time);
}

void EuclideanSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void EuclideanSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void EuclideanSequencer::ButtonClicked(ClickButton* button, double time)
{
   bool randomizeAll{ false };
   if (button == mRandomizeButton)
      randomizeAll = true;
   if (button == mRndLengthButton || randomizeAll)
      RandomizeLength(0);
   if (button == mRndOnsetsButton || randomizeAll)
      RandomizeOnset(0);
   if (button == mRndRotationButton || randomizeAll)
      RandomizeRotation(0);
   if (button == mRndOffsetButton || randomizeAll)
      RandomizeOffset(0);
   if (button == mRndNoteButton || randomizeAll)
      RandomizeNote(0);

   int ringIndex{ -1 };
   if (button == mRnd0Button)
      ringIndex = 0;
   if (button == mRnd1Button)
      ringIndex = 1;
   if (button == mRnd2Button)
      ringIndex = 2;
   if (button == mRnd3Button)
      ringIndex = 3;

   if (ringIndex != -1)  // update 1 ring only
   {
      RandomizeLength(ringIndex);
      RandomizeOnset(ringIndex);
      RandomizeRotation(ringIndex);
      RandomizeOffset(ringIndex);
      RandomizeNote(ringIndex);
   }

   if (button == mClearButton)
   {
      for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
         mEuclideanSequencerRings[i]->Clear();
   }
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

   float width, height;
   GetModuleDimensions(width, height);
   out << width;
   out << height;

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

   float width, height;
   in >> width;
   in >> height;
   Resize(width, height);

   int numRings;
   in >> numRings;
   for (size_t i = 0; i < mEuclideanSequencerRings.size() && i < numRings; ++i)
      mEuclideanSequencerRings[i]->LoadState(in);
}

void EuclideanSequencer::RandomizeLength(int ringIndex)
{
   int iFrom{ 0 };
   int iTo{ (int)mEuclideanSequencerRings.size() };
   // Check if only 1 ring is updated
   if (ringIndex > 0 && ringIndex < mEuclideanSequencerRings.size())
   {
      iFrom = ringIndex;
      iTo = ringIndex + 1;
   }
   // Update all rings or only 1 ring, depending on iFrom and iTo
   for (int i = iFrom; i < iTo; ++i)
      if (ofRandom(1) < mRndLengthChance)
         mEuclideanSequencerRings[i]->SetSteps((int)ofRandom(1, mRndLengthMax + 0.9f));
}

void EuclideanSequencer::RandomizeOnset(int ringIndex)
{
   int iFrom{ 0 };
   int iTo{ (int)mEuclideanSequencerRings.size() };
   // Check if only 1 ring is updated
   if (ringIndex > 0 && ringIndex < mEuclideanSequencerRings.size())
   {
      iFrom = ringIndex;
      iTo = ringIndex + 1;
   }
   // Update all rings or only 1 ring, depending on iFrom and iTo
   for (int i = iFrom; i < iTo; ++i)
      if (ofRandom(1) < mRndOnsetChance)
      {
         // Limit max onsets to current Steps
         int maxOnsets = mEuclideanSequencerRings[i]->GetSteps();
         // Lower max onsets if large number of Steps
         if (maxOnsets >= 12)
            maxOnsets = maxOnsets / 2;
         maxOnsets = MIN(mRndOnsetMax, maxOnsets);
         mEuclideanSequencerRings[i]->SetOnsets((int)ofRandom(1, maxOnsets +0.9f));
      }
}

void EuclideanSequencer::RandomizeRotation(int ringIndex)
{
   int iFrom{ 0 };
   int iTo{ (int)mEuclideanSequencerRings.size() };
   // Check if only 1 ring is updated
   if (ringIndex > 0 && ringIndex < mEuclideanSequencerRings.size())
   {
      iFrom = ringIndex;
      iTo = ringIndex + 1;
   }
   // Update all rings or only 1 ring, depending on iFrom and iTo
   for (int i = iFrom; i < iTo; ++i)
      if (ofRandom(1) < mRndOnsetChance)
/*       TODO: If current length of ring is smaller than mRndRotationMax, use ring length
         int minRotation =  MIN(mRndRotationMax, 
            < mRndRotationMax < EuclideanSequencerRing[i]->GetLength()

             do something with ring->GetLength
             if bigger than x, then RotationMax / 2
*/
         mEuclideanSequencerRings[i]->SetRotation((int)ofRandom(0, mRndRotationMax + 0.9f));
}

void EuclideanSequencer::RandomizeOffset(int ringIndex)
{
   int iFrom{ 0 };
   int iTo{ (int)mEuclideanSequencerRings.size() };
   // Check if only 1 ring is updated
   if (ringIndex > 0 && ringIndex < mEuclideanSequencerRings.size())
   {
      iFrom = ringIndex;
      iTo = ringIndex + 1;
   }
   // Update all rings or only 1 ring, depending on iFrom and iTo
   for (int i = iFrom; i < iTo; ++i)
      if (ofRandom(1) < mRndOnsetChance)
         if (i == 0)
         {
            mEuclideanSequencerRings[i]->SetOffset(ofRandom(0, MIN(0.04, mRndOffsetMax)));
         }
         else
         {
            mEuclideanSequencerRings[i]->SetOffset(ofRandom(0, mRndOffsetMax));
         };
}

void EuclideanSequencer::RandomizeNote(int ringIndex)
{
   int iFrom{ 0 };
   int iTo{ (int)mEuclideanSequencerRings.size() };

   // Check if only 1 ring is updated
   if (ringIndex > 0 && ringIndex < mEuclideanSequencerRings.size())
   {
      iFrom = ringIndex;
      iTo = ringIndex + 1;
   }
   // Update all rings or only 1 ring, depending on iFrom and iTo
   for (int i = iFrom; i < iTo; ++i)
      if (ofRandom(1) < mRndNoteChance)
      {
         // 0 = 0 1 2 3
         if (mRndOctaveLo == 0 && mRndOctaveHi == 0)
         {
            for (int i = 0; i < mEuclideanSequencerRings.size(); ++i)
               mEuclideanSequencerRings[i]->SetPitch(i);
         }
         else if (mRndOctaveLo >= 1.0 && mRndOctaveHi < 6.0)
         {
            int numPitchesInScale = TheScale->NumTonesInScale();
            // +2 to return octave 1
            int noteFrom = numPitchesInScale * (mRndOctaveLo + 2) + TheScale->GetScaleDegree();
            // hi - lo + 1 = range, plus noteFrom
            int noteTo = numPitchesInScale * (mRndOctaveHi - mRndOctaveLo + 1) + noteFrom;
            mEuclideanSequencerRings[i]->SetPitch(TheScale->GetPitchFromTone(ofRandom(noteFrom, noteTo)));
         }
      }
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
   int x = mIndex * 95 + 210;
   int y = 90;

   mLengthSlider = new FloatSlider(mOwner, ("steps" + ofToString(mIndex)).c_str(), x, y, 90, 15, &mLength, 0, EUCLIDEAN_SEQUENCER_MAX_STEPS, 0);
   mOnsetSlider = new FloatSlider(mOwner, ("onsets" + ofToString(mIndex)).c_str(), x, y + 20, 90, 15, &mOnset, 0, EUCLIDEAN_SEQUENCER_MAX_STEPS, 0);
   mRotationSlider = new FloatSlider(mOwner, ("rotation" + ofToString(mIndex)).c_str(), x, y + 40, 90, 15, &mRotation, -8, 8, 0);
   mOffsetSlider = new FloatSlider(mOwner, ("offset" + ofToString(mIndex)).c_str(), x, y + 60, 90, 15, &mOffset, -.25f, .25f, 2);
   mNoteSelector = new TextEntry(mOwner, ("note" + ofToString(mIndex)).c_str(), x, y + 80, 4, &mPitch, 0, 127);

   mDestinationCable = new AdditionalNoteCable();
   mDestinationCable->SetPatchCableSource(new PatchCableSource(mOwner, kConnectionType_Note));
   mDestinationCable->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
   mOwner->AddPatchCableSource(mDestinationCable->GetPatchCableSource());
   mDestinationCable->GetPatchCableSource()->SetManualPosition(x + 50, y + 105);

   // Calculate Euclidean steps
   FloatSliderUpdated(mLengthSlider, 0, 0);
}

void EuclideanSequencerRing::InitPreset(int preset)
{
   const int kPresetCount = 2;
   int defaultPresets[kPresetCount][4][4] = {
      {
      { 4, 4, 0, 0 },
      { 12, 2, 3, 1 },
      { 8, 4, 1, 2 },
      { 10, 2, 2, 3 },
      },
      {
      { 16, 4, 1, 60 },
      { 8, 6, 0, 64 },
      { 16, 2, 3, 67 },
      { 8, 2, 5, 72 },
      }
   };

   preset = MIN(preset, kPresetCount - 1);
   preset = MAX(preset, 0);
   mLengthSlider->SetValue(defaultPresets[preset][mIndex][0], gTime, true);
   mOnsetSlider->SetValue(defaultPresets[preset][mIndex][1], gTime, true);
   mRotationSlider->SetValue(defaultPresets[preset][mIndex][2], gTime, true);
   mNoteSelector->SetValue(defaultPresets[preset][mIndex][3], gTime, true);
}

void EuclideanSequencerRing::Clear()
{
   mLengthSlider->SetValue(0, gTime, true);
   mOnsetSlider->SetValue(0, gTime, true);
   mRotationSlider->SetValue(0, gTime, true);
   mOffsetSlider->SetValue(0, gTime, true);
   mNoteSelector->SetValue(mIndex, gTime, true);
}

void EuclideanSequencerRing::Draw()
{
   ofPushStyle();
   ofSetColor(128, 128, 128);
   int x = mIndex * 95 + 210;
   DrawTextNormal(NoteName(mPitch, false, true), mIndex * 95 + 210 + 60, 182);
   ofPopStyle();

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
   int idx = int(pos * mLength + .5f) % (int)mLength;

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

void EuclideanSequencerRing::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   bool forceUpdate{ false };

   if (slider == mRotationSlider)
   {
      // Do not generate a new GetEuclideanRhythm, but rotate mSteps
      // This way, manually modified onsets will remain

      // Nothing to rotate, return
      if (mLength == 0 || mOnset == 0)
         return;   // and avoid divide by zero later for: % mLength    

      std::array<float, EUCLIDEAN_SEQUENCER_MAX_STEPS> mTempSteps{};
      float newVal = (mRotationSlider->GetValue());
      int rotOffset = (int)(newVal - oldVal) % (int)mLength;

      if (rotOffset == 0)
      {
      // This is a strange situation:
      // IntSliderUpdated event was triggered for mRotationSlider,
      // but no difference between newVal and oldVal.
      // This occurs when Randomize button is used to update the Rotation
      // So force a full recalculation:

        forceUpdate = true;
      }
      else
      {
         // Regular rotation, modified onsets keep their status
         // Save current mSteps
         mTempSteps = mSteps;

         // Fill mSteps with old data using rotOffset
         for (int i = 0; i < mLength; i++)
         {
            mSteps[i] = mTempSteps[(int)(i + rotOffset + mLength) % (int)mLength]; // + mLength to avoid negative mod results
         }
      
      }
   }

   if (slider == mLengthSlider || slider == mOnsetSlider || forceUpdate)
   {
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

   if (oldStep != newStep && mSteps[newStep] > 0)
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

void EuclideanSequencerRing::SetSteps(int steps)
{
   mLength = steps;
   mLengthSlider->SetValue(mLength, gTime, true);
}

int EuclideanSequencerRing::GetSteps()
{
   return mLength;
}

void EuclideanSequencerRing::SetOnsets(int onsets)
{
   mOnset = onsets;
   mOnsetSlider->SetValue(mOnset, gTime, true);
}

void EuclideanSequencerRing::SetRotation(int rotation)
{
   mRotation = rotation;
   mRotationSlider->SetValue(mRotation, gTime, true);
}

void EuclideanSequencerRing::SetOffset(float offset)
{
   mOffset = offset;
   mOffsetSlider->SetValue(mOffset, gTime, true);
}
void EuclideanSequencerRing::SetPitch(int pitch)
{
   mPitch = pitch;
   mNoteSelector->SetValue(pitch, gTime, true);
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

   // Fill Euclidean rhythm with steps using the Euclidean music algorithm based on
   // Computer Music Design Team
   // https://web.archive.org/web/20190322182835/https:/computermusicdesign.com/simplest-euclidean-rhythm-algorithm-explained
   // and 
   // Boogie Automaticland - bohara2000
   // https://web.archive.org/web/20240329220927/https://gist.github.com/bohara2000/c1fb0076c72f786597ae0daa0a194f1f
   //
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
