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
//  FishbowlSynth.cpp
//  Bespoke
//
//  Created for fishbowl randomizer synth
//
//

#include "FishbowlSynth.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

FishbowlSynth::FishbowlSynth()
{
   // Initialize fish with random positions and velocities
   for (int i = 0; i < kMaxFish; ++i)
   {
      Fish fish;
      // Random position within bowl
      float angle = ofRandom(TWO_PI);
      float radius = ofRandom(mBowlRadius * 0.8f);
      fish.position.x = cos(angle) * radius;
      fish.position.y = sin(angle) * radius;
      
      // Random velocity
      fish.velocity.x = ofRandom(-2, 2);
      fish.velocity.y = ofRandom(-2, 2);
      
      // Random color (fish-like colors)
      fish.color = ofColor(ofRandom(200, 255), ofRandom(100, 200), ofRandom(0, 100));
      
      mFish.push_back(fish);
   }
}

void FishbowlSynth::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void FishbowlSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   INTSLIDER(mNumFishSlider, "fish", &mNumFish, 1, kMaxFish);
   FLOATSLIDER(mFishSpeedSlider, "speed", &mFishSpeed, 0.1f, 5.0f);
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 60);
   DROPDOWN(mTriggerModeSelector, "trigger", &mTriggerMode, 60);
   INTSLIDER(mMinPitchSlider, "min pitch", &mMinPitch, 0, 127);
   INTSLIDER(mMaxPitchSlider, "max pitch", &mMaxPitch, 0, 127);
   FLOATSLIDER(mProbabilitySlider, "probability", &mProbability, 0, 1);
   CHECKBOX(mModulatePanCheckbox, "mod pan", &mModulatePan);
   CHECKBOX(mModulatePressureCheckbox, "mod pressure", &mModulatePressure);
   CHECKBOX(mModulateModWheelCheckbox, "mod wheel", &mModulateModWheel);
   ENDUIBLOCK(mWidth, mHeight);
   
   mHeight += 280; // Add space for fishbowl visualization
   
   // Setup interval dropdown
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   
   // Setup trigger mode dropdown
   mTriggerModeSelector->AddLabel("interval", kTriggerMode_Interval);
   mTriggerModeSelector->AddLabel("zone cross", kTriggerMode_ZoneCross);
   mTriggerModeSelector->AddLabel("both", kTriggerMode_Both);
}

FishbowlSynth::~FishbowlSynth()
{
   TheTransport->RemoveListener(this);
}

void FishbowlSynth::Poll()
{
   IDrawableModule::Poll();
   
   if (mEnabled)
   {
      UpdateFishPhysics();
      
      // Check for zone crossing triggers
      if (mTriggerMode == kTriggerMode_ZoneCross || mTriggerMode == kTriggerMode_Both)
      {
         for (int i = 0; i < mNumFish && i < (int)mFish.size(); ++i)
         {
            Fish& fish = mFish[i];
            
            // Check if fish crosses center (vertical or horizontal line)
            bool crossedZone = (abs(fish.position.x) < 5 || abs(fish.position.y) < 5);
            
            if (crossedZone && !fish.hasTriggered)
            {
               if (ofRandom(1) < mProbability)
               {
                  TriggerNoteFromFish(fish, gTime);
               }
               fish.hasTriggered = true;
            }
            else if (!crossedZone)
            {
               fish.hasTriggered = false;
            }
         }
      }
   }
}

void FishbowlSynth::UpdateFishPhysics()
{
   for (int i = 0; i < mNumFish && i < (int)mFish.size(); ++i)
   {
      Fish& fish = mFish[i];
      
      // Update position
      fish.position += fish.velocity * mFishSpeed;
      
      // Check bowl boundary (circular)
      float distFromCenter = fish.position.length();
      if (distFromCenter > mBowlRadius)
      {
         // Bounce off wall
         ofVec2f normal = fish.position.normalized();
         fish.velocity = fish.velocity - normal * 2 * fish.velocity.dot(normal);
         fish.velocity *= 0.8f; // Some energy loss
         
         // Push back inside
         fish.position = normal * mBowlRadius;
      }
      
      // Add some random movement (fish swimming behavior)
      fish.velocity.x += ofRandom(-0.1f, 0.1f);
      fish.velocity.y += ofRandom(-0.1f, 0.1f);
      
      // Limit velocity
      float speed = fish.velocity.length();
      if (speed > 3.0f)
      {
         fish.velocity = fish.velocity.normalized() * 3.0f;
      }
      
      // Minimum velocity (fish keep swimming)
      if (speed < 0.5f)
      {
         fish.velocity = fish.velocity.normalized() * 0.5f;
      }
   }
}

void FishbowlSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   // Draw controls
   mNumFishSlider->Draw();
   mFishSpeedSlider->Draw();
   mIntervalSelector->Draw();
   mTriggerModeSelector->Draw();
   mMinPitchSlider->Draw();
   mMaxPitchSlider->Draw();
   mProbabilitySlider->Draw();
   mModulatePanCheckbox->Draw();
   mModulatePressureCheckbox->Draw();
   mModulateModWheelCheckbox->Draw();

   ofPushStyle();
   ofPushMatrix();
   
   // Draw fishbowl
   ofTranslate(mBowlCenter.x, mBowlCenter.y + 100);
   
   // Bowl outline
   ofSetColor(100, 150, 200, 100);
   ofFill();
   ofCircle(0, 0, mBowlRadius);
   
   ofSetColor(150, 200, 255, 200);
   ofNoFill();
   ofSetLineWidth(3);
   ofCircle(0, 0, mBowlRadius);
   
   // Draw center cross lines (trigger zones)
   if (mTriggerMode == kTriggerMode_ZoneCross || mTriggerMode == kTriggerMode_Both)
   {
      ofSetColor(255, 255, 255, 50);
      ofSetLineWidth(1);
      ofLine(-mBowlRadius, 0, mBowlRadius, 0);
      ofLine(0, -mBowlRadius, 0, mBowlRadius);
   }
   
   // Draw fish
   for (int i = 0; i < mNumFish && i < (int)mFish.size(); ++i)
   {
      const Fish& fish = mFish[i];
      
      ofSetColor(fish.color);
      ofFill();
      
      // Draw fish as a simple triangle pointing in direction of movement
      ofPushMatrix();
      ofTranslate(fish.position.x, fish.position.y);
      
      float angle = atan2(fish.velocity.y, fish.velocity.x);
      ofRotate(ofRadToDeg(angle));
      
      // Fish body (triangle)
      ofBeginShape();
      ofVertex(8, 0);
      ofVertex(-4, 4);
      ofVertex(-4, -4);
      ofEndShape(true);
      
      // Fish tail
      ofSetColor(fish.color.r * 0.7f, fish.color.g * 0.7f, fish.color.b * 0.7f);
      ofBeginShape();
      ofVertex(-4, 0);
      ofVertex(-8, 3);
      ofVertex(-8, -3);
      ofEndShape(true);
      
      ofPopMatrix();
   }
   
   ofPopMatrix();
   ofPopStyle();
}

void FishbowlSynth::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   if (mTriggerMode == kTriggerMode_Interval || mTriggerMode == kTriggerMode_Both)
   {
      mNoteOutput.Flush(time);
      
      // Trigger notes from random fish
      for (int i = 0; i < mNumFish && i < (int)mFish.size(); ++i)
      {
         if (ofRandom(1) < mProbability)
         {
            TriggerNoteFromFish(mFish[i], time);
         }
      }
   }
}

void FishbowlSynth::TriggerNoteFromFish(const Fish& fish, double time)
{
   int pitch = FishPositionToPitch(fish);
   int velocity = FishPositionToVelocity(fish);

   // Build modulation parameters
   ModulationParameters modulation;

   if (mModulatePan)
   {
      modulation.pan = FishPositionToPan(fish);
   }

   if (mModulatePressure)
   {
      float pressure = FishPositionToPressure(fish);
      mModulation.GetPressure(-1)->SetValue(pressure);
      modulation.pressure = mModulation.GetPressure(-1);
   }

   if (mModulateModWheel)
   {
      float modWheel = FishVelocityToModWheel(fish);
      mModulation.GetModWheel(-1)->SetValue(modWheel);
      modulation.modWheel = mModulation.GetModWheel(-1);
   }

   PlayNoteOutput(NoteMessage(time, pitch, velocity, -1, modulation));
}

int FishbowlSynth::FishPositionToPitch(const Fish& fish)
{
   // Map X position (-bowlRadius to +bowlRadius) to pitch range
   float normalizedX = (fish.position.x + mBowlRadius) / (2.0f * mBowlRadius);
   normalizedX = ofClamp(normalizedX, 0, 1);
   
   int pitch = mMinPitch + (int)(normalizedX * (mMaxPitch - mMinPitch));
   return ofClamp(pitch, 0, 127);
}

int FishbowlSynth::FishPositionToVelocity(const Fish& fish)
{
   // Map Y position to velocity (higher = louder)
   float normalizedY = (mBowlRadius - fish.position.y) / (2.0f * mBowlRadius);
   normalizedY = ofClamp(normalizedY, 0, 1);

   int velocity = 40 + (int)(normalizedY * 87); // Range 40-127
   return ofClamp(velocity, 1, 127);
}

float FishbowlSynth::FishPositionToPan(const Fish& fish)
{
   // Map X position to pan (-1 = left, 1 = right)
   float pan = fish.position.x / mBowlRadius;
   return ofClamp(pan, -1, 1);
}

float FishbowlSynth::FishPositionToPressure(const Fish& fish)
{
   // Map distance from center to pressure (0 at edge, 1 at center)
   float distFromCenter = fish.position.length();
   float pressure = 1.0f - (distFromCenter / mBowlRadius);
   return ofClamp(pressure, 0, 1);
}

float FishbowlSynth::FishVelocityToModWheel(const Fish& fish)
{
   // Map fish velocity magnitude to mod wheel (0 to 1)
   float speed = fish.velocity.length();
   float modWheel = speed / 3.0f; // Max speed is ~3.0
   return ofClamp(modWheel, 0, 1);
}

void FishbowlSynth::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void FishbowlSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void FishbowlSynth::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void FishbowlSynth::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void FishbowlSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FishbowlSynth::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

