/*
  ==============================================================================

    LooperGranulator.cpp
    Created: 13 Mar 2021 1:55:54pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "LooperGranulator.h"
#include "ModularSynth.h"
#include "Slider.h"
#include "PatchCableSource.h"
#include "Looper.h"
#include "FillSaveDropdown.h"
#include "UIControlMacros.h"

LooperGranulator::LooperGranulator()
   : mOn(false)
   , mFreeze(false)
   , mDummyPos(0)
   , mLooper(nullptr)
{
}

LooperGranulator::~LooperGranulator()
{
}

void LooperGranulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   CHECKBOX(mOnCheckbox, "on", &mOn);
   FLOATSLIDER(mGranOverlap, "overlap", &mGranulator.mGrainOverlap, .5f, MAX_GRAINS);
   FLOATSLIDER(mGranSpeed, "speed", &mGranulator.mSpeed, -3, 3);
   FLOATSLIDER(mGranLengthMs, "len ms", &mGranulator.mGrainLengthMs, 1, 200);
   FLOATSLIDER(mPosSlider, "loop pos", &mDummyPos, 0, 1); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mFreezeCheckbox, "freeze", &mFreeze); UIBLOCK_NEWLINE();
   FLOATSLIDER(mGranPosRandomize, "pos rand", &mGranulator.mPosRandomizeMs, 0, 200);
   FLOATSLIDER(mGranSpeedRandomize, "speed rand", &mGranulator.mSpeedRandomize, 0, .3f);
   CHECKBOX(mGranOctaveCheckbox, "octaves", &mGranulator.mOctaves);
   ENDUIBLOCK(mWidth, mHeight);

   mLooperCable = new PatchCableSource(this, kConnectionType_Special);
   //mLooperCable->SetManualPosition(99, 10);
   mLooperCable->AddTypeFilter("looper");
   AddPatchCableSource(mLooperCable);

   mGranPosRandomize->SetMode(FloatSlider::kSquare);
   mGranLengthMs->SetMode(FloatSlider::kSquare);
}

void LooperGranulator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (mLooper != nullptr)
      mPosSlider->SetExtents(0, mLooper->GetLoopLength());

   mOnCheckbox->Draw();
   mGranOverlap->Draw();
   mGranSpeed->Draw();
   mGranLengthMs->Draw();
   mPosSlider->Draw();
   mFreezeCheckbox->Draw();
   mGranPosRandomize->Draw();
   mGranSpeedRandomize->Draw();
   mGranOctaveCheckbox->Draw();
}

void LooperGranulator::DrawOverlay(ofRectangle bufferRect, int loopLength)
{
   if (mOn)
      mGranulator.Draw(bufferRect.x, bufferRect.y, bufferRect.width, bufferRect.height, 0, loopLength);
}

void LooperGranulator::ProcessFrame(double time, float bufferOffset, float* output)
{
   if (mLooper != nullptr)
   {
      int bufferLength;
      auto* buffer = mLooper->GetLoopBuffer(bufferLength);
      mGranulator.ProcessFrame(time, buffer, bufferLength, bufferOffset, output);
   }
}

void LooperGranulator::OnCommit()
{
   mOn = false;
   mFreeze = false;
   if (mPosSlider->GetLFO())
      mPosSlider->GetLFO()->SetLFOEnabled(false);
}

void LooperGranulator::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mLooperCable)
   {
      mLooper = dynamic_cast<Looper*>(mLooperCable->GetTarget());
      if (mLooper != nullptr)
      {
         mLooper->SetGranulator(this);
         mPosSlider->SetVar(mLooper->GetLoopPosVar());
      }
   }
}

void LooperGranulator::ButtonClicked(ClickButton* button)
{
}

void LooperGranulator::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void LooperGranulator::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void LooperGranulator::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);

   string targetPath = "";
   if (mLooperCable->GetTarget())
      targetPath = mLooperCable->GetTarget()->Path();

   moduleInfo["looper"] = targetPath;
}

void LooperGranulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("looper", moduleInfo, "", FillDropdown<Looper*>);

   SetUpFromSaveData();
}

void LooperGranulator::SetUpFromSaveData()
{
   mLooperCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("looper"), false));
}

