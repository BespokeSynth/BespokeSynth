/*
  ==============================================================================

    ValueStream.cpp
    Created: 25 Oct 2020 7:09:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ValueStream.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include <algorithm>

ValueStream::ValueStream()
   : mUIControl(nullptr)
   , mFloatSlider(nullptr)
   , mControlCable(nullptr)
   , mWidth(200)
   , mHeight(120)
   , mSpeed(1)
   , mValueDisplayPointer(0)
{
}

void ValueStream::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

ValueStream::~ValueStream()
{
   TheTransport->RemoveAudioPoller(this);
}

void ValueStream::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mSpeedSlider, "speed", &mSpeed, .1f, 5);
   ENDUIBLOCK0();
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   AddPatchCableSource(mControlCable);
}

void ValueStream::Poll()
{
}

void ValueStream::OnTransportAdvanced(float amount)
{
   if (mUIControl && mEnabled)
   {
      for (int i = 0; i < gBufferSize; ++i)
      {
         if (mFloatSlider)
            mFloatSlider->Compute(i);
         mValues[mValueDisplayPointer] = mUIControl->GetValue();
         mValueDisplayPointer = (mValueDisplayPointer + 1) % mValues.size();
      }
   }
}

void ValueStream::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mSpeedSlider->Draw();

   if (mFloatSlider)
   {
      ofBeginShape();
      for (int i = 0; i < mWidth; ++i)
      {
         float x = mWidth - i;
         int samplesAgo = int(i  / (mSpeed / 200)) + 1;
         if (samplesAgo < mValues.size())
         {
            float y = ofMap(mValues[(mValueDisplayPointer - samplesAgo + mValues.size()) % mValues.size()], mFloatSlider->GetMin(), mFloatSlider->GetMax(), mHeight - 10, 10);
            ofVertex(x, y);
         }
      }
      ofEndShape();
   }
}

void ValueStream::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (mControlCable->GetPatchCables().empty() == false)
      mUIControl = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[0]->GetTarget());
   else
      mUIControl = nullptr;

   mFloatSlider = dynamic_cast<FloatSlider*>(mUIControl);
}

void ValueStream::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void ValueStream::Resize(float w, float h)
{
   mWidth = MAX(w, 200);
   mHeight = MAX(h, 120);
}

void ValueStream::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);

   moduleInfo["uicontrol"] = mUIControl ? mUIControl->Path() : "";
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void ValueStream::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("uicontrol", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, 200, 120, 1000);
   mModuleSaveData.LoadInt("height", moduleInfo, 120, 15, 1000);

   SetUpFromSaveData();
}

void ValueStream::SetUpFromSaveData()
{
   string controlPath = mModuleSaveData.GetString("uicontrol");
   if (!controlPath.empty())
   {
      mUIControl = TheSynth->FindUIControl(controlPath);
      if (mUIControl)
         mControlCable->SetTarget(mUIControl);
   }
   else
   {
      mUIControl = nullptr;
   }

   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
}

namespace
{
   const int kSaveStateRev = 1;
}

void ValueStream::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;
}

void ValueStream::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}
