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

    ScriptStatus.cpp
    Created: 25 Apr 2020 10:51:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#if BESPOKE_WINDOWS
#define ssize_t ssize_t_undef_hack //fixes conflict with ssize_t typedefs between python and juce
#endif
#include "ScriptStatus.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include "ScriptModule.h"
#include "Prefab.h"
#if BESPOKE_WINDOWS
#undef ssize_t
#endif

#include "leathers/push"
#include "leathers/unused-value"
#include "leathers/range-loop-analysis"
#include "pybind11/embed.h"
#include "leathers/pop"

namespace py = pybind11;

ScriptStatus::ScriptStatus()
{
   ScriptModule::CheckIfPythonEverSuccessfullyInitialized();
   if ((TheSynth->IsLoadingState() || Prefab::sLoadingPrefab) && ScriptModule::sHasPythonEverSuccessfullyInitialized)
      ScriptModule::InitializePythonIfNecessary();
}

ScriptStatus::~ScriptStatus()
{
}

void ScriptStatus::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   BUTTON(mResetAll, "reset all");
   ENDUIBLOCK0();

   mWidth = 400;
   mHeight = 400;
}

void ScriptStatus::Poll()
{
   if (ScriptModule::sHasPythonEverSuccessfullyInitialized)
      ScriptModule::InitializePythonIfNecessary();

   if (!ScriptModule::sPythonInitialized)
      return;

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

   if (!ScriptModule::sHasPythonEverSuccessfullyInitialized)
   {
      DrawTextNormal("please create a \"script\" module to initialize Python", 20, 20);
      return;
   }

   mResetAll->Draw();

   DrawTextNormal(mStatus, 3, 35);
}

void ScriptStatus::OnClicked(float x, float y, bool right)
{
   if (ScriptModule::sHasPythonEverSuccessfullyInitialized)
   {
      IDrawableModule::OnClicked(x, y, right);
   }
}

void ScriptStatus::ButtonClicked(ClickButton* button)
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

void ScriptStatus::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWidth;
   out << mHeight;
}

void ScriptStatus::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWidth;
   in >> mHeight;
}
