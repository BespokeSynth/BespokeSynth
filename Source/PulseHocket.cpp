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

    PulseHocket.cpp
    Created: 22 Feb 2020 10:40:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseHocket.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"

PulseHocket::PulseHocket()
{
   mWeight[0] = 1;
   for (int i=1; i<kMaxDestinations; ++i)
      mWeight[i] = 0;
}

PulseHocket::~PulseHocket()
{
}

void PulseHocket::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   for (int i=0; i<kMaxDestinations; ++i)
   {
      FLOATSLIDER(mWeightSlider[i],("weight "+ofToString(i)).c_str(),&mWeight[i],0,1);
      mDestinationCables[i] = new PatchCableSource(this, kConnectionType_Pulse);
      mDestinationCables[i]->SetOverrideCableDir(ofVec2f(1,0));
      AddPatchCableSource(mDestinationCables[i]);
      ofRectangle rect = mWeightSlider[i]->GetRect(true);
      mDestinationCables[i]->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height/2);
   }
   ENDUIBLOCK(mWidth,mHeight);
   mWidth += 20;
   
   GetPatchCableSource()->SetEnabled(false);
}

void PulseHocket::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (int i=0; i<kMaxDestinations; ++i)
      mWeightSlider[i]->Draw();
}

void PulseHocket::OnPulse(double time, float velocity, int flags)
{
   ComputeSliders(0);
   
   float totalWeight = 0;
   for (int i=0; i<kMaxDestinations; ++i)
      totalWeight += mWeight[i];
   float random = ofRandom(totalWeight);
   
   int selectedDestination;
   for (selectedDestination=0; selectedDestination<kMaxDestinations; ++selectedDestination)
   {
      if (random <= mWeight[selectedDestination] || selectedDestination == kMaxDestinations - 1)
         break;
      random -= mWeight[selectedDestination];
   }
   
   DispatchPulse(mDestinationCables[selectedDestination], time, velocity, flags);
}

void PulseHocket::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PulseHocket::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
