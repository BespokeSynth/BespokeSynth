/*
  ==============================================================================

    ScriptStatus.cpp
    Created: 25 Apr 2020 10:51:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#if BESPOKE_WINDOWS
#define ssize_t ssize_t_undef_hack  //fixes conflict with ssize_t typedefs between python and juce
#endif
#include "ScriptStatus.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include "ScriptModule.h"
#if BESPOKE_WINDOWS
#undef ssize_t
#endif

#include "pybind11/embed.h"

namespace py = pybind11;

ScriptStatus::ScriptStatus()
: mNextUpdateTime(0)
{
   ScriptModule::InitializePythonIfNecessary();
}

ScriptStatus::~ScriptStatus()
{
}

void ScriptStatus::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   BUTTON(mResetAll,"reset all");
   ENDUIBLOCK0();
   
   mWidth = 400;
   mHeight = 400;
}

void ScriptStatus::Poll()
{
   if (gTime > mNextUpdateTime)
   {
      mStatus = py::str(py::globals());
      ofStringReplace(mStatus, ",", "\n");
      mNextUpdateTime = gTime + 100;
   }
}

void ScriptStatus::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mResetAll->Draw();
   
   DrawTextNormal(mStatus, 3, 35);
}

void ScriptStatus::ButtonClicked(ClickButton *button)
{
   ScriptModule::UninitializePython();
   ScriptModule::InitializePythonIfNecessary();
}

void ScriptStatus::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ScriptStatus::SetUpFromSaveData()
{
}

void ScriptStatus::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

namespace
{
   const int kSaveStateRev = 1;
}

void ScriptStatus::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mWidth;
   out << mHeight;
}

void ScriptStatus::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mWidth;
   in >> mHeight;
}

