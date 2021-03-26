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
   : mMouseScrollX(0)
   , mMouseScrollY(0)
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
   ENDUIBLOCK(mWidth, mHeight);
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
}

void GlobalControls::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mZoomSlider && gHoveredUIControl != mZoomSlider) //avoid bad behavior when adjusting these via mouse
   {
      //zoom in on center of screen
      float zoomAmount = (gDrawScale - oldVal) / oldVal;
      ofVec2f zoomCenter = ofVec2f(TheSynth->GetMouseX(), TheSynth->GetMouseY()) + TheSynth->GetDrawOffset();
      TheSynth->GetDrawOffset() -= zoomCenter * zoomAmount;
   }

   if (slider == mMouseScrollXSlider && gHoveredUIControl != mMouseScrollXSlider) //avoid bad behavior when adjusting these via mouse
   {
      float delta = mMouseScrollX - oldVal;
      TheSynth->MouseScrolled(-delta, 0, false);
      mMouseScrollX = 0;
   }

   if (slider == mMouseScrollYSlider && gHoveredUIControl != mMouseScrollYSlider) //avoid bad behavior when adjusting these via mouse
   {
      float delta = mMouseScrollY - oldVal;
      TheSynth->MouseScrolled(0, -delta, false);
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
   IDrawableModule::SaveLayout(moduleInfo);
}

vector<IUIControl*> GlobalControls::ControlsToNotSetDuringLoadState() const
{
   vector<IUIControl*> ignore;
   ignore.push_back(mZoomSlider);
   ignore.push_back(mXSlider);
   ignore.push_back(mYSlider);
   ignore.push_back(mMouseScrollXSlider);
   ignore.push_back(mMouseScrollYSlider);
   return ignore;
}

namespace
{
   const int kSaveStateRev = 0;
}

void GlobalControls::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;
}

void GlobalControls::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}
