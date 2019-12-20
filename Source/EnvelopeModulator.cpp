/*
  ==============================================================================

    EnvelopeModulator.cpp
    Created: 16 Nov 2017 10:28:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "EnvelopeModulator.h"
#include "PatchCableSource.h"
#include "ModularSynth.h"

EnvelopeModulator::EnvelopeModulator()
: mWidth(250)
, mHeight(102)
, mEnvelopeControl(ofVec2f(105,5),ofVec2f(mWidth-110,mHeight-10))
, mADSRViewLength(1000)
, mADSRViewLengthSlider(nullptr)
, mHasSustainStageCheckbox(nullptr)
, mSustainStageSlider(nullptr)
, mMaxSustainSlider(nullptr)
{
   mEnvelopeControl.SetViewLength(mADSRViewLength);
   mEnvelopeControl.SetADSR(&mAdsr);
   mAdsr.GetFreeReleaseLevel() = true;
   mAdsr.Set(100,100,.7f,100);
}

void EnvelopeModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   GetPatchCableSource()->SetEnabled(false);
   
   mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mTargetCable);
   
   mMinSlider = new FloatSlider(this, "low", 2, 2, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "high", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
   mADSRViewLengthSlider = new FloatSlider(this,"length", mMaxSlider, kAnchor_Below,100,15,&mADSRViewLength,100,10000);
   mHasSustainStageCheckbox = new Checkbox(this, "has sustain", mADSRViewLengthSlider, kAnchor_Below, &mAdsr.GetHasSustainStage());
   mSustainStageSlider = new IntSlider(this, "sustain stage", mHasSustainStageCheckbox, kAnchor_Below, 100, 15, &mAdsr.GetSustainStage(), 1, MAX_ADSR_STAGES-1);
   mMaxSustainSlider = new FloatSlider(this, "max sustain", mSustainStageSlider, kAnchor_Below, 100, 15, &mAdsr.GetMaxSustain(), -1, 5000);
   
   mADSRViewLengthSlider->SetMode(FloatSlider::kSquare);
   mMaxSustainSlider->SetMode(FloatSlider::kSquare);
   
   mSustainStageSlider->SetShowing(mAdsr.GetHasSustainStage());
   mMaxSustainSlider->SetShowing(mAdsr.GetHasSustainStage());
}

EnvelopeModulator::~EnvelopeModulator()
{
}

void EnvelopeModulator::DrawModule()
{
   if (Minimized())
      return;
   
   mSustainStageSlider->SetExtents(1, mAdsr.GetNumStages() - 2);
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
   mADSRViewLengthSlider->Draw();
   mHasSustainStageCheckbox->Draw();
   mSustainStageSlider->Draw();
   mMaxSustainSlider->Draw();
   
   mEnvelopeControl.Draw();
}

void EnvelopeModulator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   
   if (mNoteOutput.HasHeldNotes() == false)
   {
      mAdsr.Stop(time);
   }
   else if (velocity > 0)
   {
      mAdsr.Start(time,velocity/127.0f);
   }
}

void EnvelopeModulator::Resize(float w, float h)
{
   mWidth = MAX(w, 250);
   mHeight = MAX(h, 102);
   mEnvelopeControl.SetDimensions(ofVec2f(mWidth - 110, mHeight-10));
}

void EnvelopeModulator::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   mEnvelopeControl.OnClicked(x,y,right);
}

void EnvelopeModulator::MouseReleased()
{
   IDrawableModule::MouseReleased();
   
   mEnvelopeControl.MouseReleased();
}

bool EnvelopeModulator::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   
   mEnvelopeControl.MouseMoved(x, y);
   
   return false;
}

float EnvelopeModulator::Value(int samplesIn /*= 0*/)
{
   ComputeSliders(samplesIn);
   if (mTarget)
      return ofClamp(Interp(mAdsr.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax()), mTarget->GetMin(), mTarget->GetMax());
   return 0;
}

void EnvelopeModulator::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void EnvelopeModulator::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mHasSustainStageCheckbox)
   {
      mSustainStageSlider->SetShowing(mAdsr.GetHasSustainStage());
      mMaxSustainSlider->SetShowing(mAdsr.GetHasSustainStage());
   }
}

void EnvelopeModulator::ButtonClicked(ClickButton* button)
{
}

void EnvelopeModulator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mADSRViewLengthSlider)
      mEnvelopeControl.SetViewLength(mADSRViewLength);
}

void EnvelopeModulator::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();
   
   moduleInfo["target"] = targetPath;
}

void EnvelopeModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void EnvelopeModulator::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

namespace
{
   const int kSaveStateRev = 0;
}

void EnvelopeModulator::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mAdsr.SaveState(out);
}

void EnvelopeModulator::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mAdsr.LoadState(in);
}

