//
//  RandomNoteGenerator.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#include "RandomNoteGenerator.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"

RandomNoteGenerator::RandomNoteGenerator()
: mInterval(kInterval_16n)
, mIntervalSelector(nullptr)
, mProbability(.5f)
, mProbabilitySlider(nullptr)
, mPitch(36)
, mPitchSlider(nullptr)
, mVelocity(.8f)
, mVelocitySlider(nullptr)
, mOffset(0)
, mOffsetSlider(nullptr)
, mSkip(1)
, mSkipSlider(nullptr)
, mSkipCount(0)
{
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
}

void RandomNoteGenerator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",5,2,((int*)(&mInterval)));
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mProbabilitySlider = new FloatSlider(this,"probability",5,20,100,15,&mProbability,0,1);
   mPitchSlider = new IntSlider(this,"pitch",5,38,100,15,&mPitch,0,127);
   mVelocitySlider = new FloatSlider(this,"velocity",5,56,100,15,&mVelocity,0,1);
   mOffsetSlider = new FloatSlider(this,"offset",5,74,100,15,&mOffset,-1,1);
   mSkipSlider = new IntSlider(this,"skip",mIntervalSelector, kAnchor_Right,60,15,&mSkip,1,10);
}

RandomNoteGenerator::~RandomNoteGenerator()
{
   TheTransport->RemoveListener(this);
}

void RandomNoteGenerator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mIntervalSelector->Draw();
   mSkipSlider->Draw();
   mProbabilitySlider->Draw();
   mPitchSlider->Draw();
   mVelocitySlider->Draw();
   mOffsetSlider->Draw();
}

void RandomNoteGenerator::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;
   
   ++mSkipCount;
   
   mNoteOutput.Flush(time);
   if (mSkipCount >= mSkip)
   {
      mSkipCount = 0;
      if (mProbability >= ofRandom(1))
         PlayNoteOutput(time, mPitch, mVelocity*127, -1);
   }
}

void RandomNoteGenerator::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void RandomNoteGenerator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
   {
      TheTransport->UpdateListener(this, mInterval, OffsetInfo(mOffset/TheTransport->CountInStandardMeasure(mInterval), !K(offsetIsInMs)));
   }
}

void RandomNoteGenerator::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mPitchSlider)
      mNoteOutput.Flush(gTime);
}

void RandomNoteGenerator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
   {
      TheTransport->UpdateListener(this, mInterval, OffsetInfo(mOffset/TheTransport->CountInStandardMeasure(mInterval), !K(offsetIsInMs)));
   }
}

void RandomNoteGenerator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void RandomNoteGenerator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
