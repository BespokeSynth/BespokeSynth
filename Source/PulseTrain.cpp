/*
  ==============================================================================

    PulseTrain.cpp
    Created: 10 Mar 2020 9:15:47pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseTrain.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

PulseTrain::PulseTrain()
: mInterval(kInterval_8n)
, mIntervalSelector(nullptr)
, mLength(8)
, mLengthSlider(nullptr)
, mStep(9999)
, mResetOnStart(true)
{
   for (int i=0; i<kMaxSteps; ++i)
      mVels[i] = 1;
   
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), false);
   TheTransport->AddAudioPoller(this);
}

void PulseTrain::CreateUIControls()
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
   
   mVelocityGrid->SetGridMode(UIGrid::kMultisliderBipolar);
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

PulseTrain::~PulseTrain()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void PulseTrain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mVelocityGrid->Draw();
   
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

void PulseTrain::CheckboxUpdated(Checkbox* checkbox)
{
}

void PulseTrain::OnTransportAdvanced(float amount)
{
   PROFILER(PulseTrain);
   
   ComputeSliders(0);
}

void PulseTrain::OnTimeEvent(double time)
{
   Step(time, 1, 0);
}

void PulseTrain::OnPulse(double time, float velocity, int flags)
{
   mStep = 0;
   Step(time, velocity, kPulseFlag_Reset);
}

void PulseTrain::Step(double time, float velocity, int flags)
{
   if (!mEnabled)
      return;
   
   bool isReset = (flags & kPulseFlag_Reset);
   if (mStep >= mLength && !isReset)
      return;
   
   ++mStep;
   
   if (isReset)
      mStep = 0;
   
   if (mStep < mLength)
   {
      float v = mVels[mStep] * velocity;
      
      int flags = 0;
      if (mResetOnStart && mStep == 0)
         flags = kPulseFlag_Reset;
      
      if (v > 0)
      {
         DispatchPulse(GetPatchCableSource(), time, v, flags);
         
         if (mStep < kIndividualStepCables)
            DispatchPulse(mStepCables[mStep], time, v, flags);
      }
   }
   
   mVelocityGrid->SetHighlightCol(time, mStep);
}

void PulseTrain::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = 52;
}

void PulseTrain::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mVelocityGrid->TestClick(x, y, right);
}

void PulseTrain::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool PulseTrain::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool PulseTrain::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   mVelocityGrid->NotifyMouseScrolled(x,y,scrollX,scrollY);
   return false;
}

void PulseTrain::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      TheTransport->UpdateListener(this, mInterval);
}

void PulseTrain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void PulseTrain::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLengthSlider)
   {
      mVelocityGrid->SetGrid(mLength, 1);
      GridUpdated(mVelocityGrid, 0, 0, 0, 0);
   }
}

void PulseTrain::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
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

void PulseTrain::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mVelocityGrid->SaveState(out);
}

void PulseTrain::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mVelocityGrid->LoadState(in);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);
}

void PulseTrain::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void PulseTrain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseTrain::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
