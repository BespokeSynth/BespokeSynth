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

//static
ScriptModule* ScriptModule::sCurrentScriptModule = nullptr;

ScriptModule::ScriptModule()
: mCodeEntry(nullptr)
, mRunButton(nullptr)
, mScheduledPulseTime(-1)
, mMostRecentRunTime(0)
{
   mPythonGlobals = py::globals();
   
   for (int i=0; i<kScheduledNoteOutputBufferSize; ++i)
      mScheduledNoteOutput[i].time = -1;
   
   for (int i=0; i<kPendingNoteInputBufferSize; ++i)
      mPendingNoteInput[i].time = -1;
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
   if (mScheduledPulseTime != -1)
   {
      RunCode(mScheduledPulseTime, "on_pulse()");
      mScheduledPulseTime = -1;
   }
   
   for (int i=0; i<kScheduledNoteOutputBufferSize; ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          gTime + 50 > mScheduledNoteOutput[i].time)
      {
         PlayNoteOutput(mScheduledNoteOutput[i].time, mScheduledNoteOutput[i].pitch, mScheduledNoteOutput[i].velocity);
         mScheduledNoteOutput[i].time = -1;
      }
   }
   
   for (int i=0; i<kPendingNoteInputBufferSize; ++i)
   {
      if (mPendingNoteInput[i].time != -1 &&
          gTime + 50 > mPendingNoteInput[i].time)
      {
         RunCode(mPendingNoteInput[i].time, "on_note("+ofToString(mPendingNoteInput[i].pitch)+", "+ofToString(mPendingNoteInput[i].velocity)+")");
         mPendingNoteInput[i].time = -1;
      }
   }
}

PYBIND11_EMBEDDED_MODULE(bespoke, m) {
   // `m` is a `py::module` which is used to bind functions and classes
   m.def("play_note", [](int pitch, int velocity)
   {
      ScriptModule::sCurrentScriptModule->PlayNoteFromScript(pitch, velocity);
   });
   m.def("schedule_note", [](float measureTime, int pitch, int velocity)
   {
      ScriptModule::sCurrentScriptModule->PlayNoteFromScript(pitch, velocity, measureTime);
   });
   m.def("set_value", [](string path, float value)
   {
      IUIControl* control = TheSynth->FindUIControl(path);
      if (control != nullptr)
         control->SetValue(value);
   });
   m.def("get_measure_time", []()
   {
      return TheTransport->GetMeasure() + TheTransport->GetMeasurePos();
   });
}

void ScriptModule::PlayNoteFromScript(int pitch, int velocity, float measureTime /*= -1*/)
{
   double time = mMostRecentRunTime;
   if (measureTime != -1)
      time = mMostRecentRunTime + (measureTime - (TheTransport->GetMeasure() + TheTransport->GetMeasurePos())) * TheTransport->MsPerBar();
   
   if (time <= mMostRecentRunTime)
      PlayNoteOutput(time, pitch, velocity);
   else
      ScheduleNote(time, pitch, velocity);
}

void ScriptModule::ScheduleNote(double time, int pitch, int velocity)
{
   for (int i=0; i<kScheduledNoteOutputBufferSize; ++i)
   {
      if (mScheduledNoteOutput[i].time == -1)
      {
         mScheduledNoteOutput[i].time = time;
         mScheduledNoteOutput[i].pitch = pitch;
         mScheduledNoteOutput[i].velocity = velocity;
         break;
      }
   }
}

void ScriptModule::ButtonClicked(ClickButton* button)
{
   if (button == mRunButton)
   {
      mCodeEntry->Publish();
      RunScript(gTime);
   }
}

void ScriptModule::ExecuteCode(string code)
{
   RunScript(gTime);
}

void ScriptModule::OnPulse(float amount, int samplesTo, int flags)
{
   mScheduledPulseTime = gTime + samplesTo * gInvSampleRateMs;
}

//INoteReceiver
void ScriptModule::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationParameters modulation /*= ModulationParameters()*/)
{
   for (int i=0; i<kPendingNoteInputBufferSize; ++i)
   {
      if (mPendingNoteInput[i].time == -1)
      {
         mPendingNoteInput[i].time = time;
         mPendingNoteInput[i].pitch = pitch;
         mPendingNoteInput[i].velocity = velocity;
         break;
      }
   }
}

void ScriptModule::RunScript(double time)
{
   //should only be called from main thread
   
   py::exec("import bespoke", mPythonGlobals);
   RunCode(time, mCodeEntry->GetText());
}

void ScriptModule::RunCode(double time, string code)
{
   //should only be called from main thread
   
   sCurrentScriptModule = this;
   mMostRecentRunTime = time;

   try
   {
      //ofLog() << "****";
      //ofLog() << (string)py::str(mPythonGlobals);
      
      FixUpCode(code);
      //ofLog() << code;
      py::exec(code, mPythonGlobals);
      
      //ofLog() << "&&&&";
      //ofLog() << (string)py::str(mPythonGlobals);
      
      //line-by-line attempt
      /*py::object scope = py::module::import("__main__").attr("__dict__");
      py::exec("import bespoke", scope);
      vector<string> lines = ofSplitString(mCodeEntry->GetText(), "\n");
      for (size_t i=0; i<lines.size(); ++i)
         py::exec(lines[i], scope);*/
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

void ScriptModule::FixUpCode(string& code)
{
   ofStringReplace(code, "on_pulse(", "on_pulse__"+Path()+"(");
   ofStringReplace(code, "on_note(", "on_note__"+Path()+"(");
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
