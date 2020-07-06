/*
  ==============================================================================

    PulseSequence.cpp
    Created: 21 Oct 2018 11:26:09pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseSequence.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

PulseSequence::PulseSequence()
: mInterval(kInterval_8n)
, mIntervalSelector(nullptr)
, mHasExternalPulseSource(false)
, mLength(8)
, mLengthSlider(nullptr)
, mStep(0)
, mAdvanceBackwardButton(nullptr)
, mAdvanceForwardButton(nullptr)
{
   for (int i=0; i<kMaxSteps; ++i)
      mVels[i] = 1;
   
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), false);
   TheTransport->AddAudioPoller(this);
}

void PulseSequence::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLengthSlider = new IntSlider(this,"length",3,2,96,15,&mLength,1,kMaxSteps);
   mIntervalSelector = new DropdownList(this,"interval",mLengthSlider,kAnchor_Right,(int*)(&mInterval));
   
   mVelocityGrid = new UIGrid(3,20,174,15,mLength,1, this);
   
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
   mIntervalSelector->AddLabel("none", kInterval_None);
   
   mAdvanceBackwardButton = new ClickButton(this,"<",mIntervalSelector,kAnchor_Right);
   mAdvanceForwardButton = new ClickButton(this,">",mAdvanceBackwardButton,kAnchor_Right);
   
   mVelocityGrid->SetGridMode(UIGrid::kMultislider);
   mVelocityGrid->SetClickClearsToZero(false);
   mVelocityGrid->SetListener(this);
   for (int i=0; i<kMaxSteps; ++i)
      mVelocityGrid->SetVal(i, 0, mVels[i], !K(notifyListener));
   
   for (int i=0; i<kIndividualStepCables; ++i)
   {
      mStepCables[i] = new PatchCableSource(this, kConnectionType_Pulse);
      mStepCables[i]->SetOverrideCableDir(ofVec2f(0,1));
      AddPatchCableSource(mStepCables[i]);
   }
}

PulseSequence::~PulseSequence()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void PulseSequence::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mIntervalSelector->SetShowing(!mHasExternalPulseSource);
   
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mVelocityGrid->Draw();
   mAdvanceBackwardButton->Draw();
   mAdvanceForwardButton->Draw();
   
   for (int i=0; i<kIndividualStepCables; ++i)
   {
      if (i < mLength)
      {
         ofVec2f pos = mVelocityGrid->GetCellPosition(i, 0) + mVelocityGrid->GetPosition(true);
         pos.x += mVelocityGrid->GetWidth() / float(mLength) * .5f;
         pos.y += mVelocityGrid->GetHeight() + 8;
         mStepCables[i]->SetManualPosition(pos.x, pos.y);
         mStepCables[i]->SetEnabled(true);
      }
      else
      {
         mStepCables[i]->SetEnabled(false);
      }
   }
}

void PulseSequence::CheckboxUpdated(Checkbox* checkbox)
{
}

void PulseSequence::OnTransportAdvanced(float amount)
{
   PROFILER(PulseSequence);
   
   ComputeSliders(0);
}

void PulseSequence::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void PulseSequence::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;
   Step(time, velocity, flags);
}

void PulseSequence::Step(double time, float velocity, int flags)
{
   if (!mEnabled)
      return;
   
   int direction = 1;
   if (flags & kPulseFlag_Backward)
      direction = -1;
   
   mStep = (mStep + direction + mLength) % mLength;
   
   if (flags & kPulseFlag_Reset)
      mStep = 0;
   else if (flags & kPulseFlag_Random)
      mStep = rand() % mLength;
   
   if (flags & kPulseFlag_SyncToTransport)
   {
      int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop()/TheTransport->GetTimeSigBottom();
      int numMeasures = ceil(float(mLength) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      mStep = (TheTransport->GetQuantized(time, mInterval) + measure * stepsPerMeasure) % mLength;
   }
   
   float v = mVels[mStep] * velocity;
   
   if (v > 0)
   {
      DispatchPulse(GetPatchCableSource(), time, v, 0);
      
      if (mStep < kIndividualStepCables)
         DispatchPulse(mStepCables[mStep], time, v, 0);
   }
   
   mVelocityGrid->SetHighlightCol(mStep);
}

void PulseSequence::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = 52;
}

void PulseSequence::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mVelocityGrid->TestClick(x, y, right);
}

void PulseSequence::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool PulseSequence::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool PulseSequence::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   mVelocityGrid->NotifyMouseScrolled(x,y,scrollX,scrollY);
   return false;
}

void PulseSequence::ButtonClicked(ClickButton* button)
{
   if (button == mAdvanceBackwardButton)
      Step(1, 0, kPulseFlag_Backward);
   if (button == mAdvanceForwardButton)
      Step(1, 0, 0);
}

void PulseSequence::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      TheTransport->UpdateListener(this, mInterval);
}

void PulseSequence::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void PulseSequence::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLengthSlider)
   {
      mVelocityGrid->SetGrid(mLength, 1);
      GridUpdated(mVelocityGrid, 0, 0, 0, 0);
   }
}

void PulseSequence::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mVelocityGrid)
   {
      for (int i=0; i<mVelocityGrid->GetCols(); ++i)
         mVels[i] = mVelocityGrid->GetVal(i,0);
   }
}

namespace
{
   const int kSaveStateRev = 1;
}

void PulseSequence::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mVelocityGrid->SaveState(out);
}

void PulseSequence::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mVelocityGrid->LoadState(in);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);
}

void PulseSequence::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void PulseSequence::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseSequence::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
