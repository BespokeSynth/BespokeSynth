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

ValueStream::ValueStream()
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
   FLOATSLIDER(mSpeedSlider, "speed", &mSpeed, .4f, 5);
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
         int samplesAgo = int(i / (mSpeed / 200)) + 1;
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
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void ValueStream::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("width", moduleInfo, 200, 120, 1000);
   mModuleSaveData.LoadInt("height", moduleInfo, 120, 15, 1000);

   SetUpFromSaveData();
}

void ValueStream::SetUpFromSaveData()
{
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
}

void ValueStream::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void ValueStream::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());
}
