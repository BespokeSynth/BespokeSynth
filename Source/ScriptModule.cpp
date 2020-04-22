/*
  ==============================================================================

    ScriptModule.cpp
    Created: 19 Apr 2020 1:52:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ScriptModule.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include "pybind11/embed.h"

namespace py = pybind11;

//static
ScriptModule* ScriptModule::sCurrentScriptModule = nullptr;

ScriptModule::ScriptModule()
: mCodeEntry(nullptr)
, mRunButton(nullptr)
, mScheduledRunTime(-1)
, mMostRecentRunTime(0)
{
}

ScriptModule::~ScriptModule()
{
}

void ScriptModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   UICONTROL_CUSTOM(mCodeEntry, new CodeEntry(this,"code",2,2,400,300));
   BUTTON(mRunButton, "run");
   ENDUIBLOCK(mWidth, mHeight);
}

void ScriptModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mCodeEntry->Draw();
   mRunButton->Draw();
}

void ScriptModule::Poll()
{
   if (mScheduledRunTime != -1)
   {
      RunScript(mScheduledRunTime);
      mScheduledRunTime = -1;
   }
}

PYBIND11_EMBEDDED_MODULE(bespoke, m) {
   // `m` is a `py::module` which is used to bind functions and classes
   m.def("play_note", [](int pitch, int velocity)
   {
      ScriptModule::sCurrentScriptModule->PlayNoteFromScript(pitch, velocity);
   });
   m.def("set_value", [](string path, float value)
   {
      IUIControl* control = TheSynth->FindUIControl(path);
      if (control != nullptr)
         control->SetValue(value);
   });
}

void ScriptModule::PlayNoteFromScript(int pitch, int velocity)
{
   PlayNoteOutput(mScheduledRunTime, pitch, velocity);
}

void ScriptModule::ButtonClicked(ClickButton* button)
{
   if (button == mRunButton)
   {
      mCodeEntry->Publish();
      RunScript(gTime);
   }
}

void ScriptModule::OnPulse(float amount, int samplesTo, int flags)
{
   mScheduledRunTime = gTime + samplesTo * gInvSampleRateMs;
}

void ScriptModule::RunScript(double time)
{
   //should only be called from main thread
   
   //ofLog() << "running script!";
   
   sCurrentScriptModule = this;
   mMostRecentRunTime = time;

   try
   {
      //auto interface = py::module::import("scriptmodule_interface");
      py::exec("import bespoke\n"+mCodeEntry->GetText());
   }
   catch (pybind11::error_already_set &e)
   {
      ofLog() << "python execution exception (error_already_set): " << e.what();
   }
   catch (const std::exception &e)
   {
      ofLog() << "python execution exception: " << e.what();
   }
   
   sCurrentScriptModule = nullptr;
}

void ScriptModule::GetModuleDimensions(int& w, int& h)
{
   w = mWidth;
   h = mHeight;
}

void ScriptModule::Resize(float w, float h)
{
   int entryW, entryH;
   mCodeEntry->GetDimensions(entryW, entryH);
   mCodeEntry->SetDimensions(entryW + w - mWidth, entryH + h - mHeight);
   mRunButton->SetPosition(mRunButton->GetPosition(true).x, mRunButton->GetPosition(true).y + h - mHeight);
   mWidth = w;
   mHeight = h;
}

void ScriptModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ScriptModule::SetUpFromSaveData()
{
}

void ScriptModule::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

namespace
{
   const int kSaveStateRev = 1;
}

void ScriptModule::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mWidth;
   out << mHeight;
}

void ScriptModule::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   float w, h;
   in >> w;
   in >> h;
   Resize(w, h);
}
