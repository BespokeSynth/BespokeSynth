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

    GlobalControls.cpp
    Created: 26 Sep 2020 11:34:18am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "GlobalControls.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

GlobalControls::GlobalControls()
{
}

GlobalControls::~GlobalControls()
{
}

void GlobalControls::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mZoomSlider, "zoom", &gDrawScale, .1f, 8.0f);
   FLOATSLIDER(mXSlider, "x pos", &(TheSynth->GetDrawOffset().x), -10000, 10000);
   FLOATSLIDER(mYSlider, "y pos", &(TheSynth->GetDrawOffset().y), -10000, 10000);
   FLOATSLIDER(mMouseScrollXSlider, "scroll x", &mMouseScrollX, -100, 100);
   FLOATSLIDER(mMouseScrollYSlider, "scroll y", &mMouseScrollY, -100, 100);
   FLOATSLIDER(mBackgroundLissajousRSlider, "lissajous r", &ModularSynth::sBackgroundLissajousR, 0, 1);
   FLOATSLIDER(mBackgroundLissajousGSlider, "lissajous g", &ModularSynth::sBackgroundLissajousG, 0, 1);
   FLOATSLIDER(mBackgroundLissajousBSlider, "lissajous b", &ModularSynth::sBackgroundLissajousB, 0, 1);
   FLOATSLIDER(mBackgroundRSlider, "background r", &ModularSynth::sBackgroundR, 0, 1);
   FLOATSLIDER(mBackgroundGSlider, "background g", &ModularSynth::sBackgroundG, 0, 1);
   FLOATSLIDER(mBackgroundBSlider, "background b", &ModularSynth::sBackgroundB, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);
}

void GlobalControls::Poll()
{
   ComputeSliders(0);
}

void GlobalControls::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mZoomSlider->Draw();
   mXSlider->Draw();
   mYSlider->Draw();
   mMouseScrollXSlider->Draw();
   mMouseScrollYSlider->Draw();
   mBackgroundLissajousRSlider->Draw();
   mBackgroundLissajousGSlider->Draw();
   mBackgroundLissajousBSlider->Draw();
   mBackgroundRSlider->Draw();
   mBackgroundGSlider->Draw();
   mBackgroundBSlider->Draw();
}

void GlobalControls::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mZoomSlider && gHoveredUIControl != mZoomSlider) //avoid bad behavior when adjusting these via mouse
   {
      //zoom in on center of screen
      float zoomAmount = (gDrawScale - oldVal) / oldVal;
      ofVec2f zoomCenter = ofVec2f(TheSynth->GetMouseX(GetOwningContainer()), TheSynth->GetMouseY(GetOwningContainer())) + TheSynth->GetDrawOffset();
      TheSynth->GetDrawOffset() -= zoomCenter * zoomAmount;
   }

   if (slider == mMouseScrollXSlider && gHoveredUIControl != mMouseScrollXSlider) //avoid bad behavior when adjusting these via mouse
   {
      float delta = mMouseScrollX - oldVal;
      TheSynth->MouseScrolled(-delta, 0, false, false, false);
      mMouseScrollX = 0;
   }

   if (slider == mMouseScrollYSlider && gHoveredUIControl != mMouseScrollYSlider) //avoid bad behavior when adjusting these via mouse
   {
      float delta = mMouseScrollY - oldVal;
      TheSynth->MouseScrolled(0, -delta, false, false, false);
      mMouseScrollY = 0;
   }
}

void GlobalControls::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

void GlobalControls::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void GlobalControls::SetUpFromSaveData()
{
}

void GlobalControls::SaveLayout(ofxJSONElement& moduleInfo)
{
}

std::vector<IUIControl*> GlobalControls::ControlsToNotSetDuringLoadState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mZoomSlider);
   ignore.push_back(mXSlider);
   ignore.push_back(mYSlider);
   ignore.push_back(mMouseScrollXSlider);
   ignore.push_back(mMouseScrollYSlider);
   return ignore;
}

void GlobalControls::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void GlobalControls::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());
}
