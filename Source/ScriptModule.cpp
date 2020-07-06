/*
  ==============================================================================

    ScriptModule.cpp
    Created: 19 Apr 2020 1:52:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#if BESPOKE_WINDOWS
#define ssize_t ssize_t_undef_hack  //fixes conflict with ssize_t typedefs between python and juce
#endif
#include "ScriptModule.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Scale.h"
#include "UIControlMacros.h"
#if BESPOKE_WINDOWS
#undef ssize_t
#endif

#include "ScriptModule_PythonInterface.i"

#include "pybind11/embed.h"
#include "pybind11/stl.h"

namespace py = pybind11;

//static
std::vector<ScriptModule*> ScriptModule::sScriptModules;
//static
ScriptModule* ScriptModule::sLastLineExecutedModule = nullptr;
//static
double ScriptModule::sMostRecentRunTime = 0;

ScriptModule::ScriptModule()
: mCodeEntry(nullptr)
, mRunButton(nullptr)
, mStopButton(nullptr)
, mLoadScriptIndex(-1)
, mA(0)
, mB(0)
, mC(0)
, mD(0)
, mNextLineToExecute(-1)
{
   InitializePythonIfNecessary();
   
   Stop();
   
   mScriptModuleIndex = sScriptModules.size();
   sScriptModules.push_back(this);
   
   Transport::sDoEventLookahead = true;   //scripts require lookahead to be able to schedule on time
}

ScriptModule::~ScriptModule()
{
}

void ScriptModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   DROPDOWN(mLoadScriptSelector, "loadscript", &mLoadScriptIndex, 120); UIBLOCK_SHIFTRIGHT();
   BUTTON(mLoadScriptButton,"load"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveScriptButton,"save as"); UIBLOCK_NEWLINE();
   UICONTROL_CUSTOM(mCodeEntry, new CodeEntry(UICONTROL_BASICS("code"),500,300));
   BUTTON(mRunButton, "run"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mStopButton, "stop"); UIBLOCK_NEWLINE();
   FLOATSLIDER(mASlider, "a", &mA, 0, 1); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mBSlider, "b", &mB, 0, 1); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mCSlider, "c", &mC, 0, 1); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mDSlider, "d", &mD, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);
}

static bool sPythonInitialized = false;

void ScriptModule::UninitializePython()
{
   if (sPythonInitialized)
      py::finalize_interpreter();
   sPythonInitialized = false;
}

void ScriptModule::InitializePythonIfNecessary()
{
   if (!sPythonInitialized)
   {
      py::initialize_interpreter();
      py::exec("import bespoke", py::globals());
      py::exec("import module", py::globals());
      py::exec("import scriptmodule", py::globals());
      py::exec("import random", py::globals());
      py::exec("import math", py::globals());
   }
   sPythonInitialized = true;
}

void ScriptModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mLoadScriptSelector->Draw();
   mLoadScriptButton->Draw();
   mSaveScriptButton->Draw();
   mCodeEntry->Draw();
   mRunButton->Draw();
   mStopButton->Draw();
   mASlider->Draw();
   mBSlider->Draw();
   mCSlider->Draw();
   mDSlider->Draw();
   
   if (mLastError != "")
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      ofVec2f errorPos = mStopButton->GetPosition(true);
      errorPos.x += 60;
      errorPos.y += 12;
      DrawTextNormal(mLastError, errorPos.x, errorPos.y);
   }
   
   mLineExecuteTracker.Draw(mCodeEntry, 0, ofColor::green);
   mNotePlayTracker.Draw(mCodeEntry, 1, IDrawableModule::GetColor(kModuleType_Note));
   mMethodCallTracker.Draw(mCodeEntry, 1, IDrawableModule::GetColor(kModuleType_Other));
   mUIControlTracker.Draw(mCodeEntry, 1, IDrawableModule::GetColor(kModuleType_Modulator));
   
   for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          //mScheduledNoteOutput[i].velocity > 0 &&
          gTime + 50 < mScheduledNoteOutput[i].time)
         DrawTimer(mScheduledNoteOutput[i].lineNum, mScheduledNoteOutput[i].startTime, mScheduledNoteOutput[i].time, IDrawableModule::GetColor(kModuleType_Note), mScheduledNoteOutput[i].velocity > 0);
   }
   
   for (size_t i=0; i<mScheduledMethodCall.size(); ++i)
   {
      if (mScheduledMethodCall[i].time != -1 &&
          gTime + 50 < mScheduledMethodCall[i].time)
         DrawTimer(mScheduledMethodCall[i].lineNum, mScheduledMethodCall[i].startTime, mScheduledMethodCall[i].time, IDrawableModule::GetColor(kModuleType_Other), true);
   }
   
   for (size_t i=0; i<mScheduledUIControlValue.size(); ++i)
   {
      if (mScheduledUIControlValue[i].time != -1 &&
          gTime + 50 < mScheduledUIControlValue[i].time)
         DrawTimer(mScheduledUIControlValue[i].lineNum, mScheduledUIControlValue[i].startTime, mScheduledUIControlValue[i].time, IDrawableModule::GetColor(kModuleType_Modulator), true);
   }
   
   ofPushStyle();
   for (size_t i=0; i<mPrintDisplay.size(); ++i)
   {
      float fadeMs = 500;
      if (gTime - mPrintDisplay[i].time > 0 && gTime - mPrintDisplay[i].time < fadeMs)
      {
         ofSetColor(ofColor::white, 255*(1-(gTime - mPrintDisplay[i].time)/fadeMs));
         ofVec2f linePos = mCodeEntry->GetLinePos(mPrintDisplay[i].lineNum, K(end));
         DrawTextNormal(mPrintDisplay[i].text, linePos.x + 10, linePos.y + 15);
      }
      else
      {
         mPrintDisplay[i].time = -1;
      }
   }
   
   for (size_t i=0; i<mUIControlModifications.size(); ++i)
   {
      float fadeMs = 500;
      if (gTime - mUIControlModifications[i].time > 0 && gTime - mUIControlModifications[i].time < fadeMs)
      {
         ofSetColor(IDrawableModule::GetColor(kModuleType_Modulator), 255*(1-(gTime - mUIControlModifications[i].time)/fadeMs));
         ofVec2f linePos = mCodeEntry->GetLinePos(mUIControlModifications[i].lineNum, K(end));
         DrawTextNormal(ofToString(mUIControlModifications[i].value), linePos.x + 10, linePos.y + 15);
      }
      else
      {
         mUIControlModifications[i].time = -1;
      }
   }
   
   ofPopStyle();
}

void ScriptModule::DrawModuleUnclipped()
{
   ofPushStyle();
   if (mDrawDebug)
   {
      string debugText = mLastRunLiteralCode;
      
      for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
      {
         if (mScheduledNoteOutput[i].time != -1 &&
             gTime + 50 < mScheduledNoteOutput[i].time)
            debugText += "\nP:"+ofToString(mScheduledNoteOutput[i].pitch) + " V:" + ofToString(mScheduledNoteOutput[i].velocity) + ", " + ofToString(mScheduledNoteOutput[i].time) + " " + ofToString(mScheduledNoteOutput[i].startTime) + ", line:" + ofToString(mScheduledNoteOutput[i].lineNum);
      }
      
      for (size_t i=0; i<mScheduledMethodCall.size(); ++i)
      {
         if (mScheduledMethodCall[i].time != -1 &&
             gTime + 50 < mScheduledMethodCall[i].time)
            debugText += "\n"+mScheduledMethodCall[i].method + ", " + ofToString(mScheduledMethodCall[i].time) + " " + ofToString(mScheduledMethodCall[i].startTime) + " " + ofToString(mScheduledMethodCall[i].lineNum);
      }
      
      for (size_t i=0; i<mScheduledUIControlValue.size(); ++i)
      {
         if (mScheduledUIControlValue[i].time != -1 &&
             gTime + 50 < mScheduledUIControlValue[i].time)
            debugText += "\n"+ string(mScheduledUIControlValue[i].control->Name()) + ": " + ofToString(mScheduledUIControlValue[i].value) + ", " + ofToString(mScheduledUIControlValue[i].time) + " " + ofToString(mScheduledUIControlValue[i].startTime) + " " + ofToString(mScheduledUIControlValue[i].lineNum);
      }
      
      string lineNumbers = "";
      vector<string> lines = ofSplitString(mLastRunLiteralCode, "\n");
      for (size_t i=0; i<lines.size(); ++i)
      {
         lineNumbers += ofToString(i+1)+"\n";
      }
      
      ofSetColor(100,100,100);
      DrawTextNormal(lineNumbers, mWidth+5, 0);
      ofSetColor(255,255,255);
      DrawTextNormal(debugText, mWidth + 30, 0);
   }
   
   for (size_t i=0; i<mUIControlModifications.size(); ++i)
   {
      float fadeMs = 200;
      if (gTime - mUIControlModifications[i].time > 0 && gTime - mUIControlModifications[i].time < fadeMs)
      {
         ofSetColor(IDrawableModule::GetColor(kModuleType_Modulator), 100*(1-(gTime - mUIControlModifications[i].time)/fadeMs));
         
         ofVec2f linePos = mCodeEntry->GetLinePos(mUIControlModifications[i].lineNum, false);
      
         ofPushMatrix();
         ofTranslate(-mX, -mY);
         ofSetLineWidth(1);
         ofLine(mX + linePos.x + 11, mY + linePos.y + 10, mUIControlModifications[i].position.x, mUIControlModifications[i].position.y);
         ofPopMatrix();
      }
   }
   ofPopStyle();
}

void ScriptModule::DrawTimer(int lineNum, double startTime, double endTime, ofColor color, bool filled)
{
   ofVec2f linePos = mCodeEntry->GetLinePos(lineNum, false);
   linePos.x += 11;
   linePos.y += 10;
   float t = (gTime - startTime) / (endTime - startTime);
   if (t > 0 && t < 1)
   {
      const float kRadius = 5;
      ofPushStyle();
      if (filled)
         ofSetColor(color);
      else
         ofSetColor(color * .5f);
      ofNoFill();
      ofCircle(linePos.x, linePos.y, kRadius);
      if (filled)
         ofFill();
      ofCircle(linePos.x + sin(t * TWO_PI) * kRadius, linePos.y - cos(t * TWO_PI) * kRadius, 2);
      ofPopStyle();
   }
}

void ScriptModule::Poll()
{
   double time = gTime;
   
   for (size_t i=0; i<mScheduledPulseTimes.size(); ++i)
   {
      if (mScheduledPulseTimes[i] != -1)
      {
         double runTime = mScheduledPulseTimes[i];
         mScheduledPulseTimes[i] = -1;
         if (mLastError == "")
         {
            //if (runTime < time)
            //   ofLog() << "trying to run script triggered by pulse too late!";
            RunCode(runTime, "on_pulse()");
         }
      }
   }
   
   for (size_t i=0; i<mPendingNoteInput.size(); ++i)
   {
      if (mPendingNoteInput[i].time != -1 &&
          time + TheTransport->GetEventLookaheadMs() > mPendingNoteInput[i].time)
      {
         if (mLastError == "")
         {
            //if (mPendingNoteInput[i].time < time)
            //   ofLog() << "trying to run script triggered by note too late!";
            RunCode(mPendingNoteInput[i].time, "on_note("+ofToString(mPendingNoteInput[i].pitch)+", "+ofToString(mPendingNoteInput[i].velocity)+")");
         }
         mPendingNoteInput[i].time = -1;
      }
   }
   
   for (size_t i=0; i<mScheduledUIControlValue.size(); ++i)
   {
      if (mScheduledUIControlValue[i].time != -1 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledUIControlValue[i].time)
      {
         AdjustUIControl(mScheduledUIControlValue[i].control, mScheduledUIControlValue[i].value, mScheduledUIControlValue[i].lineNum);
         mScheduledUIControlValue[i].time = -1;
      }
   }
   
   //note offs first
   for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          mScheduledNoteOutput[i].velocity == 0 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledNoteOutput[i].time)
      {
         PlayNote(mScheduledNoteOutput[i].time, mScheduledNoteOutput[i].pitch, mScheduledNoteOutput[i].velocity, mScheduledNoteOutput[i].pan, mScheduledNoteOutput[i].lineNum);
         mScheduledNoteOutput[i].time = -1;
      }
   }
   
   //then note ons
   for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          mScheduledNoteOutput[i].velocity != 0 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledNoteOutput[i].time)
      {
         PlayNote(mScheduledNoteOutput[i].time, mScheduledNoteOutput[i].pitch, mScheduledNoteOutput[i].velocity, mScheduledNoteOutput[i].pan, mScheduledNoteOutput[i].lineNum);
         mScheduledNoteOutput[i].time = -1;
      }
   }
   
   for (size_t i=0; i<mScheduledMethodCall.size(); ++i)
   {
      if (mScheduledMethodCall[i].time != -1 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledMethodCall[i].time)
      {
         RunCode(mScheduledMethodCall[i].time, mScheduledMethodCall[i].method);
         mMethodCallTracker.AddEvent(mScheduledMethodCall[i].lineNum);
         mScheduledMethodCall[i].time = -1;
      }
   }
}

//static
float ScriptModule::GetScriptMeasureTime()
{
   return TheTransport->GetMeasureTime(sMostRecentRunTime);
}

//static
float ScriptModule::GetTimeSigRatio()
{
   return float(TheTransport->GetTimeSigTop()) / TheTransport->GetTimeSigBottom();
}

void ScriptModule::PlayNoteFromScript(int pitch, int velocity, float pan)
{
   PlayNote(sMostRecentRunTime, pitch, velocity, pan, mNextLineToExecute);
}

void ScriptModule::PlayNoteFromScriptAfterDelay(int pitch, int velocity, float delayMeasureTime, float pan)
{
   double time = sMostRecentRunTime + delayMeasureTime * TheTransport->MsPerBar();
   if (velocity == 0)
      time -= gBufferSize * gInvSampleRateMs + 1;  //TODO(Ryan) hack to make note offs happen a buffer early... figure out why scheduled lengths are longer than it takes to get the next pulse of the same interval
   
   //ofLog() << "ScriptModule::PlayNoteFromScriptAfterDelay() " << velocity << " " << time << " " << sMostRecentRunTime << " " << (time - sMostRecentRunTime);
   
   if (time <= sMostRecentRunTime)
   {
      if (time < gTime)
         ofLog() << "script is trying to play a note in the past!";
      PlayNote(time, pitch, velocity, pan, mNextLineToExecute);
   }
   else
   {
      ScheduleNote(time, pitch, velocity, pan);
   }
}

void ScriptModule::ScheduleNote(double time, int pitch, int velocity, float pan)
{
   for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time == -1)
      {
         mScheduledNoteOutput[i].time = time;
         mScheduledNoteOutput[i].startTime = sMostRecentRunTime;
         mScheduledNoteOutput[i].pitch = pitch;
         mScheduledNoteOutput[i].velocity = velocity;
         mScheduledNoteOutput[i].pan = pan;
         mScheduledNoteOutput[i].lineNum = mNextLineToExecute;
         break;
      }
   }
}

void ScriptModule::ScheduleMethod(string method, float delayMeasureTime)
{
   for (size_t i=0; i<mScheduledMethodCall.size(); ++i)
   {
      if (mScheduledMethodCall[i].time == -1)
      {
         double time = sMostRecentRunTime + delayMeasureTime * TheTransport->MsPerBar();
         
         mScheduledMethodCall[i].time = time;
         mScheduledMethodCall[i].startTime = sMostRecentRunTime;
         mScheduledMethodCall[i].method = method;
         mScheduledMethodCall[i].lineNum = mNextLineToExecute;
         break;
      }
   }
}

void ScriptModule::ScheduleUIControlValue(IUIControl* control, float value, float delayMeasureTime)
{
   for (size_t i=0; i<mScheduledUIControlValue.size(); ++i)
   {
      if (mScheduledUIControlValue[i].time == -1)
      {
         double time = sMostRecentRunTime + delayMeasureTime * TheTransport->MsPerBar();
         
         mScheduledUIControlValue[i].time = time;
         mScheduledUIControlValue[i].startTime = sMostRecentRunTime;
         mScheduledUIControlValue[i].control = control;
         mScheduledUIControlValue[i].value = value;
         mScheduledUIControlValue[i].lineNum = mNextLineToExecute;
         break;
      }
   }
}

void ScriptModule::HighlightLine(int lineNum, int scriptModuleIndex)
{
   sLastLineExecutedModule = sScriptModules[scriptModuleIndex];
   sScriptModules[scriptModuleIndex]->mNextLineToExecute = lineNum;
   sScriptModules[scriptModuleIndex]->mLineExecuteTracker.AddEvent(lineNum);
}

void ScriptModule::PrintText(string text)
{
   for (size_t i=0; i<mPrintDisplay.size(); ++i)
   {
      if (mPrintDisplay[i].time == -1 || mPrintDisplay[i].lineNum == mNextLineToExecute)
      {
         mPrintDisplay[i].time = gTime;
         mPrintDisplay[i].text = text;
         mPrintDisplay[i].lineNum = mNextLineToExecute;
         break;
      }
   }
}

IUIControl* ScriptModule::GetUIControl(string path)
{
   IUIControl* control;
   if (ofIsStringInString(path, "~"))
      control = TheSynth->FindUIControl(path);
   else
      control = TheSynth->FindUIControl(Path() + "~" + path);
   
   return control;
}

void ScriptModule::AdjustUIControl(IUIControl* control, float value, int lineNum)
{
   control->SetValue(value);
   
   mUIControlTracker.AddEvent(lineNum);
   
   for (size_t i=0; i<mUIControlModifications.size(); ++i)
   {
      if (mUIControlModifications[i].time == -1 || mUIControlModifications[i].lineNum == lineNum)
      {
         mUIControlModifications[i].time = gTime;
         mUIControlModifications[i].position = control->GetRect().getCenter();
         mUIControlModifications[i].value = value;
         mUIControlModifications[i].lineNum = lineNum;
         break;
      }
   }
}

void ScriptModule::PlayNote(double time, int pitch, int velocity, float pan, int lineNum)
{
   /*if (velocity > 0)
   {
      //run through any scheduled note offs for this pitch
      for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
      {
         if (mScheduledNoteOutput[i].time != -1 &&
             mScheduledNoteOutput[i].pitch == pitch &&
             mScheduledNoteOutput[i].velocity == 0)
         {
            PlayNote(time, pitch, 0, 0, lineNum);
            mScheduledNoteOutput[i].time = -1;
         }
      }
   }*/
   
   //ofLog() << "ScriptModule::PlayNote() " << velocity << " " << time;
   ModulationParameters modulation;
   modulation.pan = pan;
   PlayNoteOutput(time, pitch, velocity, -1, modulation);
   //if (velocity > 0)
      mNotePlayTracker.AddEvent(lineNum, ofToString(pitch) + " " + ofToString(velocity) + " " + ofToString(pan,1));
}

void ScriptModule::ButtonClicked(ClickButton* button)
{
   if (button == mRunButton)
   {
      mCodeEntry->Publish();
      RunScript(gTime);
   }
   
   if (button == mStopButton)
      Stop();
   
   if (button == mSaveScriptButton)
   {
      FileChooser chooser("Save script as...", File(ofToDataPath("scripts/script.py")));
      if (chooser.browseForFileToSave(true))
      {
         string path = chooser.getResult().getFullPathName().toStdString();
         
         File resourceFile (path);
         TemporaryFile tempFile (resourceFile);
         FileOutputStream output (tempFile.getFile());

         if (!output.openedOk())
         {
            DBG("FileOutputStream didn't open correctly ...");
            return;
         }
             
         output.setNewLineString("\n");
         output.writeString(mCodeEntry->GetText());
         output.flush(); // (called explicitly to force an fsync on posix)

         if (output.getStatus().failed())
         {
            DBG("An error occurred in the FileOutputStream");
            return;
         }

         bool success = tempFile.overwriteTargetFileWithTemporary();
         if (!success)
         {
            DBG("An error occurred writing the file");
            return;
         }
         
         RefreshScriptFiles();
         
         for (size_t i=0; i<mScriptFilePaths.size(); ++i)
         {
            if (mScriptFilePaths[i] == path)
            {
               mLoadScriptIndex = (int)i;
               break;
            }
         }
      }
   }
   
   if (button == mLoadScriptButton)
   {
      if (mLoadScriptIndex >= 0 && mLoadScriptIndex < (int)mScriptFilePaths.size())
      {
         File resourceFile = File(mScriptFilePaths[mLoadScriptIndex]);
         
         if (!resourceFile.existsAsFile())
         {
            DBG("File doesn't exist ...");
            return;
         }

         unique_ptr<FileInputStream> input(resourceFile.createInputStream());

         if (!input->openedOk())
         {
            DBG("Failed to open file");
            return;
         }
             
         mCodeEntry->SetText(input->readString().toStdString());
      }
   }
}

void ScriptModule::DropdownClicked(DropdownList* list)
{
   if (list == mLoadScriptSelector)
      RefreshScriptFiles();
}

void ScriptModule::RefreshScriptFiles()
{
   DirectoryIterator dir(File(ofToDataPath("scripts")), false);
   mScriptFilePaths.clear();
   mLoadScriptSelector->Clear();
   while(dir.next())
   {
      File file = dir.getFile();
      if (file.getFileExtension() ==  ".py")
      {
         mLoadScriptSelector->AddLabel(file.getFileName().toStdString(), mScriptFilePaths.size());
         mScriptFilePaths.push_back(file.getFullPathName().toStdString());
      }
   }
}

void ScriptModule::ExecuteCode(string code)
{
   RunScript(gTime);
}

void ScriptModule::OnPulse(double time, float velocity, int flags)
{
   for (size_t i=0; i<mScheduledPulseTimes.size(); ++i)
   {
      if (mScheduledPulseTimes[i] == -1)
      {
         mScheduledPulseTimes[i] = time;
         break;
      }
   }
}

//INoteReceiver
void ScriptModule::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationParameters modulation /*= ModulationParameters()*/)
{
   for (size_t i=0; i<mPendingNoteInput.size(); ++i)
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

string ScriptModule::GetThisName()
{
   return "this__"+ofToString(mScriptModuleIndex);
}

void ScriptModule::RunScript(double time)
{
   //should only be called from main thread
   py::exec(GetThisName()+" = scriptmodule.get_this("+ofToString(mScriptModuleIndex)+")", py::globals());
   string code = mCodeEntry->GetText();
   vector<string> lines = ofSplitString(code, "\n");
   code = "";
   for (size_t i=0; i<lines.size(); ++i)
   {
      if (ShouldDisplayLineExecutionPre(i > 0 ? lines[i-1] : "", lines[i]))
         code += GetIndentation(lines[i])+"this.highlight_line("+ofToString(i)+","+ofToString(mScriptModuleIndex)+")               ###instrumentation###\n";
      code += lines[i]+"\n";
      if (ShouldDisplayLineExecutionPost(lines[i]))
         code += GetIndentation(lines[i])+"   this.highlight_line("+ofToString(i)+","+ofToString(mScriptModuleIndex)+")               ###instrumentation###\n";
   }
   FixUpCode(code);
   mLastRunLiteralCode = code;
   RunCode(time, code);
}

void ScriptModule::RunCode(double time, string code)
{
   //should only be called from main thread
   
   sMostRecentRunTime = time;
   mNextLineToExecute = -1;
   ComputeSliders(time);

   try
   {
      //ofLog() << "****";
      //ofLog() << (string)py::str(mPythonGlobals);
      
      FixUpCode(code);
      //ofLog() << code;
      py::exec(code, py::globals());
      
      //ofLog() << "&&&&";
      //ofLog() << (string)py::str(mPythonGlobals);
      
      mCodeEntry->SetError(false);
      mLastError = "";
   }
   catch (pybind11::error_already_set &e)
   {
      ofLog() << "python execution exception (error_already_set): " << e.what();
      
      if (mNextLineToExecute == -1) //this script hasn't executed yet
         sLastLineExecutedModule = this;
      
      sLastLineExecutedModule->mLastError = (string)py::str(e.type()) + ": "+ (string)py::str(e.value());
      
      int lineNumber = sLastLineExecutedModule->mNextLineToExecute;
      if (lineNumber == -1)
      {
         string errorString = (string)py::str(e.value());
         const string lineTextLabel = " line ";
         const char* lineTextPos = strstr(errorString.c_str(), lineTextLabel.c_str());
         if (lineTextPos != nullptr)
         {
            try
            {
               size_t start = lineTextPos + lineTextLabel.length() - errorString.c_str();
               size_t len = errorString.size() - 1 - start;
               string lineNumberText = errorString.substr(start, len);
               int rawLineNumber = stoi(lineNumberText);
               int realLineNumber = rawLineNumber - 1;
               
               vector<string> lines = ofSplitString(sLastLineExecutedModule->mLastRunLiteralCode, "\n");
               for (size_t i=0; i<lines.size() && i < rawLineNumber; ++i)
               {
                  if (ofIsStringInString(lines[i], "###instrumentation###"))
                     --realLineNumber;
               }
               
               lineNumber = realLineNumber;
            }
            catch(std::exception const & e)
            {
            }
         }
         //PyErr_NormalizeException(&e.type().ptr(),&e.value().ptr(),&e.trace().ptr());

         /*char *msg, *file, *text;
         int line, offset;

         int res = PyArg_ParseTuple(e.value().ptr(),"s(siis)",&msg,&file,&line,&offset,&text);

         //ofLog() << e.value().
         
         if (res > 0)
         {
            PyObject* line_no = PyObject_GetAttrString(e.value().ptr(),"lineno");
            PyObject* line_no_str = PyObject_Str(line_no);
            PyObject* line_no_unicode = PyUnicode_AsEncodedString(line_no_str,"utf-8", "Error");
            char *actual_line_no = PyBytes_AsString(line_no_unicode);  // Line number
            ofLog() << actual_line_no;
         }*/
         
         /*PyTracebackObject* trace = (PyTracebackObject*)e.trace().ptr();
         if (trace != nullptr)
         {
            while (trace->tb_next)
               trace = trace->tb_next;
            PyFrameObject* frame = trace->tb_frame;
            while (frame)
            {
               if (frame->f_back != nullptr)
                  lineNumber += PyFrame_GetLineNumber(frame);
               if (frame->f_back == nullptr)
                  lineNumber -= PyFrame_GetLineNumber(frame);  //take away root frame? not sure.
               frame = frame->f_back;
            }
         }*/
      }
      
      sLastLineExecutedModule->mCodeEntry->SetError(true, lineNumber);
   }
   catch (const std::exception &e)
   {
      ofLog() << "python execution exception: " << e.what();
   }
}

void ScriptModule::FixUpCode(string& code)
{
   ofStringReplace(code, "on_pulse(", "on_pulse__"+Path()+"(");
   ofStringReplace(code, "on_note(", "on_note__"+Path()+"(");
   ofStringReplace(code, "this.", GetThisName()+".");
}

void ScriptModule::GetFirstAndLastCharacter(string line, char& first, char& last)
{
   bool hasFirstCharacter = false;
   for (size_t i = 0; i < line.length(); ++i)
   {
      char c = line[i];
      if (c != ' ')
      {
         if (!hasFirstCharacter)
         {
            hasFirstCharacter = true;
            first = c;
         }
         last = c;
      }
   }
}

bool ScriptModule::ShouldDisplayLineExecutionPre(string priorLine, string line)
{
   if (!IsNonWhitespace(line))
      return false;
   
   char firstCharacter;
   char lastCharacter;
   
   GetFirstAndLastCharacter(priorLine, firstCharacter, lastCharacter);
   if (lastCharacter == ',')
      return false;
   
   GetFirstAndLastCharacter(line, firstCharacter, lastCharacter);
   
   if (firstCharacter == '#')
      return false;
   if (lastCharacter == ':' && (ofIsStringInString(line, "else") || ofIsStringInString(line, "elif")))
      return false;
   return true;
}

bool ScriptModule::ShouldDisplayLineExecutionPost(string line)
{
   return false;
   //return ofIsStringInString(line, "def ");
   /*
   char firstCharacter;
   char lastCharacter;
   GetFirstAndLastCharacter(line, firstCharacter, lastCharacter);
   
   if (firstCharacter == '#')
      return false;
   return lastCharacter == ':';*/
}

string ScriptModule::GetIndentation(string line)
{
   string ret;
   for (size_t i = 0; i < line.length(); ++i)
   {
      if (line[i] == ' ')
         ret += ' ';
      else
         break;
   }
   return ret;
}

bool ScriptModule::IsNonWhitespace(string line)
{
   for (size_t i = 0; i < line.length(); ++i)
   {
      if (line[i] != ' ')
         return true;
   }
   return false;
}

void ScriptModule::Stop()
{
   for (size_t i=0; i<mScheduledPulseTimes.size(); ++i)
      mScheduledPulseTimes[i] = -1;
   
   for (size_t i=0; i<mScheduledNoteOutput.size(); ++i)
      mScheduledNoteOutput[i].time = -1;
   
   for (size_t i=0; i<mScheduledMethodCall.size(); ++i)
      mScheduledMethodCall[i].time = -1;
   
   for (size_t i=0; i<mScheduledUIControlValue.size(); ++i)
      mScheduledUIControlValue[i].time = -1;
   
   for (size_t i=0; i<mPendingNoteInput.size(); ++i)
      mPendingNoteInput[i].time = -1;
   
   for (size_t i=0; i<mPrintDisplay.size(); ++i)
      mPrintDisplay[i].time = -1;
}

void ScriptModule::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

void ScriptModule::Resize(float w, float h)
{
   float entryW, entryH;
   mCodeEntry->GetDimensions(entryW, entryH);
   mCodeEntry->SetDimensions(entryW + w - mWidth, entryH + h - mHeight);
   mRunButton->SetPosition(mRunButton->GetPosition(true).x, mRunButton->GetPosition(true).y + h - mHeight);
   mStopButton->SetPosition(mStopButton->GetPosition(true).x, mStopButton->GetPosition(true).y + h - mHeight);
   mASlider->SetPosition(mASlider->GetPosition(true).x, mASlider->GetPosition(true).y + h - mHeight);
   mBSlider->SetPosition(mBSlider->GetPosition(true).x, mBSlider->GetPosition(true).y + h - mHeight);
   mCSlider->SetPosition(mCSlider->GetPosition(true).x, mCSlider->GetPosition(true).y + h - mHeight);
   mDSlider->SetPosition(mDSlider->GetPosition(true).x, mDSlider->GetPosition(true).y + h - mHeight);
   mWidth = w;
   mHeight = h;
}

bool ScriptModule::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   mCodeEntry->NotifyMouseScrolled(x,y,scrollX,scrollY);
   return false;
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

void ScriptModule::LineEventTracker::Draw(CodeEntry* codeEntry, int style, ofColor color)
{
   ofPushStyle();
   ofFill();
   for (size_t i=0; i<mText.size(); ++i)
   {
      float alpha = style == 0 ? 200 : 150;
      float fadeMs = style == 0 ? 200 : 150;
      if (gTime - mTimes[i] > 0 && gTime - mTimes[i] < fadeMs)
      {
         ofSetColor(color, alpha*(1-(gTime - mTimes[i])/fadeMs));
         ofVec2f linePos = codeEntry->GetLinePos(i, false);
         if (style == 0)
            ofRect(linePos.x + 1, linePos.y + 3, 4, codeEntry->GetCharHeight(), L(corner,0));
         if (style == 1)
            ofCircle(linePos.x + 11, linePos.y + 10, 5);
         
         if (mText[i] != "")
         {
            ofVec2f linePos = codeEntry->GetLinePos(i, K(end));
            DrawTextNormal(mText[i], linePos.x + 10, linePos.y + 15);
         }
      }
   }
   ofPopStyle();
}
