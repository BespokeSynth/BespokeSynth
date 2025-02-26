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

    ScriptModule.cpp
    Created: 19 Apr 2020 1:52:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#if BESPOKE_WINDOWS
#define ssize_t ssize_t_undef_hack //fixes conflict with ssize_t typedefs between python and juce
#endif
#include "ScriptModule.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Scale.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"
#include "Prefab.h"
#include "VersionInfo.h"
#if BESPOKE_WINDOWS
#undef ssize_t
#endif

#include "ScriptModule_PythonInterface.i"

#include "leathers/push"
#include "leathers/unused-value"
#include "leathers/range-loop-analysis"
#include "pybind11/embed.h"
#include "pybind11/stl.h"
#include "leathers/pop"
#include "juce_cryptography/juce_cryptography.h"

namespace py = pybind11;
using namespace juce;

//static
std::vector<ScriptModule*> ScriptModule::sScriptModules;
//static
std::list<ScriptModule*> ScriptModule::sScriptsRequestingInitExecution;
//static
ScriptModule* ScriptModule::sMostRecentLineExecutedModule = nullptr;
//static
ScriptModule* ScriptModule::sPriorExecutedModule = nullptr;
//static
double ScriptModule::sMostRecentRunTime = 0;
//static
std::string ScriptModule::sBackgroundTextString = "";
//static
float ScriptModule::sBackgroundTextSize = 30;
//static
ofVec2f ScriptModule::sBackgroundTextPos;
//static
ofColor ScriptModule::sBackgroundTextColor = ofColor::white;
//static
bool ScriptModule::sPythonInitialized = false;
//static
bool ScriptModule::sHasPythonEverSuccessfullyInitialized =
#ifdef BESPOKE_PORTABLE_PYTHON
true;
#else
false;
#endif
//static
bool ScriptModule::sHasLoadedUntrustedScript = false;

//static
ofxJSONElement ScriptModule::sStyleJSON;

ScriptModule::ScriptModule()
{
   CheckIfPythonEverSuccessfullyInitialized();
   if ((TheSynth->IsLoadingState() || Prefab::sLoadingPrefab) && sHasPythonEverSuccessfullyInitialized)
      InitializePythonIfNecessary();

   Reset();

   mScriptModuleIndex = sScriptModules.size();
   sScriptModules.push_back(this);

   OSCReceiver::addListener(this);
}

ScriptModule::~ScriptModule()
{
}

void ScriptModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mLoadScriptSelector, "loadscript", &mLoadScriptIndex, 120);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mLoadScriptButton, "load");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mSaveScriptButton, "save as");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShowReferenceButton, "?");
   UIBLOCK_NEWLINE();
   UICONTROL_CUSTOM(mCodeEntry, new CodeEntry(UICONTROL_BASICS("code"), 500, 300));
   BUTTON(mRunButton, "run");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mStopButton, "stop");
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mASlider, "a", &mA, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mBSlider, "b", &mB, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mCSlider, "c", &mC, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mDSlider, "d", &mD, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);

   UIBLOCK0();
   BUTTON(mTrustScriptButton, "yes, I trust this script, load it");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mDontTrustScriptButton, "no, I don't trust this script, abort load");
   ENDUIBLOCK0();

   mPythonInstalledConfirmButton = new ClickButton(this, "yes, I have Python installed", 20, 100);

   RefreshStyleFiles();

   if (!sStyleJSON.empty())
      mCodeEntry->SetStyleFromJSON(sStyleJSON[0u]);
}

void ScriptModule::UninitializePython()
{
   if (sPythonInitialized)
      py::finalize_interpreter();
   sPythonInitialized = false;
}

namespace
{
   // Py_SetPythonHome()'s signature varies depending on Python version. This converts to the string type we need.
   std::string toPythonHome(const std::string& s, void (*)(char*)) { return s; }
   std::wstring toPythonHome(const std::string& s, void (*)(const wchar_t*)) { return juce::String{ s }.toWideCharPointer(); }
}

//static
void ScriptModule::InitializePythonIfNecessary()
{
   if (!sPythonInitialized)
   {
#ifdef BESPOKE_PORTABLE_PYTHON
      static const auto pythonHomeUtf8{ ofToFactoryPath("python") };
      static auto PYTHONHOME{ toPythonHome(pythonHomeUtf8, Py_SetPythonHome) };
      Py_SetPythonHome(PYTHONHOME.data());
#endif
      py::initialize_interpreter();
#ifdef BESPOKE_PORTABLE_PYTHON
      py::exec(std::string{ "import sys; sys.executable = '" } + pythonHomeUtf8 + "/" BESPOKE_PORTABLE_PYTHON "'; del sys");
#endif
      py::exec(GetBootstrapImportString(), py::globals());

      CodeEntry::OnPythonInit();
   }
   sPythonInitialized = true;

   if (!sHasPythonEverSuccessfullyInitialized)
   {
      File(ofToDataPath("internal/python_" + std::string(Bespoke::PYTHON_VERSION) + "_installed")).create();
      sHasPythonEverSuccessfullyInitialized = true;
   }
}

//static
void ScriptModule::CheckIfPythonEverSuccessfullyInitialized()
{
   if (!sHasPythonEverSuccessfullyInitialized)
   {
      if (File(ofToDataPath("internal/python_" + std::string(Bespoke::PYTHON_VERSION) + "_installed")).existsAsFile())
         sHasPythonEverSuccessfullyInitialized = true;
   }
}

void ScriptModule::OnClicked(float x, float y, bool right)
{
   if (!sHasPythonEverSuccessfullyInitialized)
   {
      mPythonInstalledConfirmButton->TestClick(x, y, right);
   }
   else
   {
      IDrawableModule::OnClicked(x, y, right);
   }
}

void ScriptModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (!sHasPythonEverSuccessfullyInitialized)
   {
      //DrawTextNormal("please ensure that you have Python 3.8 installed\nif you do not have Python 3.8 installed, Bespoke may crash\n\nclick to continue...", 20, 20);
      juce::String pythonVersionRev = Bespoke::PYTHON_VERSION;
      juce::String pythonVersionMinor = pythonVersionRev.substring(0, pythonVersionRev.lastIndexOfChar('.'));
      if (pythonVersionRev.lastIndexOfChar('.') == -1)
         pythonVersionMinor = "***ERROR***";
      DrawTextNormal("this version of bespoke was built with Python " + pythonVersionRev.toStdString() + "\n" +
                     "please ensure that you have some flavor of Python " + pythonVersionMinor.toStdString() + " installed.\n" +
                     "(not an older or newer version!)\n\n" +
                     "if you do not, bespoke will crash!",
                     20, 20);

      mCodeEntry->SetShowing(false);

      mPythonInstalledConfirmButton->SetLabel(("yes, I have Python " + pythonVersionMinor.toStdString() + " installed").c_str());
      mPythonInstalledConfirmButton->Draw();

      return;
   }

   mTrustScriptButton->SetShowing(mIsScriptUntrusted);
   mDontTrustScriptButton->SetShowing(mIsScriptUntrusted);
   mLoadScriptSelector->SetShowing(!mIsScriptUntrusted);
   mLoadScriptButton->SetShowing(!mIsScriptUntrusted);
   mSaveScriptButton->SetShowing(!mIsScriptUntrusted);
   mShowReferenceButton->SetShowing(!mIsScriptUntrusted);

   mPythonInstalledConfirmButton->SetShowing(false);
   mCodeEntry->SetShowing(true);

   mLoadScriptSelector->Draw();
   mLoadScriptButton->Draw();
   mSaveScriptButton->Draw();
   mShowReferenceButton->Draw();
   mCodeEntry->Draw();
   mRunButton->Draw();
   mStopButton->Draw();
   mASlider->Draw();
   mBSlider->Draw();
   mCSlider->Draw();
   mDSlider->Draw();

   if (mIsScriptUntrusted)
   {
      mTrustScriptButton->Draw();
      mDontTrustScriptButton->Draw();
      ofPushStyle();
      ofFill();

      ofRectangle buttonRect = mTrustScriptButton->GetRect(K(local));
      ofSetColor(0, 255, 0, 80);
      ofRect(buttonRect);

      buttonRect = mDontTrustScriptButton->GetRect(K(local));
      ofSetColor(255, 0, 0, 80);
      ofRect(buttonRect);

      ofPopStyle();
   }

   if (mLastError != "")
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      ofVec2f errorPos = mStopButton->GetPosition(true);
      errorPos.x += 60;
      errorPos.y += 12;
      DrawTextNormal(mLastError, errorPos.x, errorPos.y);
   }

   mLineExecuteTracker.Draw(mCodeEntry, 0, ofColor::green);
   mNotePlayTracker.Draw(mCodeEntry, 1, IDrawableModule::GetColor(kModuleCategory_Note));
   mMethodCallTracker.Draw(mCodeEntry, 1, IDrawableModule::GetColor(kModuleCategory_Other));
   mUIControlTracker.Draw(mCodeEntry, 1, IDrawableModule::GetColor(kModuleCategory_Modulator));

   for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          //mScheduledNoteOutput[i].velocity > 0 &&
          gTime + 50 < mScheduledNoteOutput[i].time)
         DrawTimer(mScheduledNoteOutput[i].lineNum, mScheduledNoteOutput[i].startTime, mScheduledNoteOutput[i].time, IDrawableModule::GetColor(kModuleCategory_Note), mScheduledNoteOutput[i].velocity > 0);
   }

   for (size_t i = 0; i < mScheduledMethodCall.size(); ++i)
   {
      if (mScheduledMethodCall[i].time != -1 &&
          gTime + 50 < mScheduledMethodCall[i].time)
         DrawTimer(mScheduledMethodCall[i].lineNum, mScheduledMethodCall[i].startTime, mScheduledMethodCall[i].time, IDrawableModule::GetColor(kModuleCategory_Other), true);
   }

   for (size_t i = 0; i < mScheduledUIControlValue.size(); ++i)
   {
      if (mScheduledUIControlValue[i].time != -1 &&
          gTime + 50 < mScheduledUIControlValue[i].time)
         DrawTimer(mScheduledUIControlValue[i].lineNum, mScheduledUIControlValue[i].startTime, mScheduledUIControlValue[i].time, IDrawableModule::GetColor(kModuleCategory_Modulator), true);
   }

   ofPushStyle();
   for (size_t i = 0; i < mPrintDisplay.size(); ++i)
   {
      if (mPrintDisplay[i].time == -1)
         continue;

      float fadeMs = 500;
      if (gTime - mPrintDisplay[i].time >= 0 && gTime - mPrintDisplay[i].time < fadeMs)
      {
         ofSetColor(ofColor::white, 255 * (1 - (gTime - mPrintDisplay[i].time) / fadeMs));
         ofVec2f linePos = mCodeEntry->GetLinePos(mPrintDisplay[i].lineNum, K(end));
         DrawTextNormal(mPrintDisplay[i].text, linePos.x + 10, linePos.y + 15);
      }
      else
      {
         mPrintDisplay[i].time = -1;
      }
   }

   for (size_t i = 0; i < mUIControlModifications.size(); ++i)
   {
      if (mUIControlModifications[i].time == -1)
         continue;

      float fadeMs = 500;
      if (gTime - mUIControlModifications[i].time >= 0 && gTime - mUIControlModifications[i].time < fadeMs)
      {
         ofSetColor(IDrawableModule::GetColor(kModuleCategory_Modulator), 255 * (1 - (gTime - mUIControlModifications[i].time) / fadeMs));
         ofVec2f linePos = mCodeEntry->GetLinePos(mUIControlModifications[i].lineNum, K(end));
         DrawTextNormal(ofToString(mUIControlModifications[i].value), linePos.x + 10, linePos.y + 15);
      }
      else
      {
         mUIControlModifications[i].time = -1;
      }
   }
   ofPopStyle();

   if (CodeEntry::HasJediNotInstalledWarning())
   {
      ofPushStyle();
      ofRectangle buttonRect = mShowReferenceButton->GetRect(true);
      ofFill();
      ofSetColor(255, 255, 0);
      float x = buttonRect.getMaxX() + 10;
      float y = buttonRect.getCenter().y;
      ofCircle(x, y, 6);
      ofSetColor(0, 0, 0);
      DrawTextBold("!", x - 2, y + 5, 15);
      ofPopStyle();

      if (mShowJediWarning)
         TheSynth->SetNextDrawTooltip("warning: jedi is not installed, so scripting autocomplete will not work. to add autocomplete functionality, install jedi, which you can likely do with the command 'pip install jedi' in a terminal window");
   }
}

void ScriptModule::DrawModuleUnclipped()
{
   if (Minimized() || IsVisible() == false)
      return;

   mCodeEntry->RenderOverlay();

   ofPushStyle();
   if (mDrawDebug)
   {
      std::string debugText = mLastRunLiteralCode;

      for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
      {
         if (mScheduledNoteOutput[i].time != -1 &&
             gTime + 50 < mScheduledNoteOutput[i].time)
            debugText += "\nP:" + ofToString(mScheduledNoteOutput[i].pitch) + " V:" + ofToString(mScheduledNoteOutput[i].velocity) + ", " + ofToString(mScheduledNoteOutput[i].time) + " " + ofToString(mScheduledNoteOutput[i].startTime) + ", line:" + ofToString(mScheduledNoteOutput[i].lineNum);
      }

      for (size_t i = 0; i < mScheduledMethodCall.size(); ++i)
      {
         if (mScheduledMethodCall[i].time != -1 &&
             gTime + 50 < mScheduledMethodCall[i].time)
            debugText += "\n" + mScheduledMethodCall[i].method + ", " + ofToString(mScheduledMethodCall[i].time) + " " + ofToString(mScheduledMethodCall[i].startTime) + " " + ofToString(mScheduledMethodCall[i].lineNum);
      }

      for (size_t i = 0; i < mScheduledUIControlValue.size(); ++i)
      {
         if (mScheduledUIControlValue[i].time != -1 &&
             gTime + 50 < mScheduledUIControlValue[i].time)
            debugText += "\n" + std::string(mScheduledUIControlValue[i].control->Name()) + ": " + ofToString(mScheduledUIControlValue[i].value) + ", " + ofToString(mScheduledUIControlValue[i].time) + " " + ofToString(mScheduledUIControlValue[i].startTime) + " " + ofToString(mScheduledUIControlValue[i].lineNum);
      }

      std::string lineNumbers = "";
      std::vector<std::string> lines = ofSplitString(mLastRunLiteralCode, "\n");
      for (size_t i = 0; i < lines.size(); ++i)
      {
         lineNumbers += ofToString(i + 1) + "\n";
      }

      ofSetColor(100, 100, 100);
      DrawTextNormal(lineNumbers, mWidth + 5, 0);
      ofSetColor(255, 255, 255);
      DrawTextNormal(debugText, mWidth + 30, 0);
   }

   for (size_t i = 0; i < mUIControlModifications.size(); ++i)
   {
      if (mUIControlModifications[i].time == -1)
         continue;

      float fadeMs = 200;
      if (gTime - mUIControlModifications[i].time >= 0 && gTime - mUIControlModifications[i].time < fadeMs)
      {
         ofSetColor(IDrawableModule::GetColor(kModuleCategory_Modulator), 100 * (1 - (gTime - mUIControlModifications[i].time) / fadeMs));

         ofVec2f linePos = mCodeEntry->GetLinePos(mUIControlModifications[i].lineNum, false);

         ofPushMatrix();
         ofTranslate(-mX, -mY);
         ofSetLineWidth(1);
         ofLine(mX + linePos.x + 11, mY + linePos.y + 10, mUIControlModifications[i].position.x, mUIControlModifications[i].position.y);
         ofPopMatrix();
      }
   }

   if (mDrawBoundModuleConnections && mBoundModuleConnections.size() > 0)
   {
      for (size_t i = 0; i < mBoundModuleConnections.size(); ++i)
      {
         if (mBoundModuleConnections[i].mTarget == nullptr)
            continue;
         if (mBoundModuleConnections[i].mTarget->IsDeleted())
         {
            mBoundModuleConnections[i].mTarget = nullptr;
            continue;
         }

         ofVec2f linePos = mCodeEntry->GetLinePos(mBoundModuleConnections[i].mLineIndex, false);

         ofSetColor(IDrawableModule::GetColor(kModuleCategory_Other), 30);
         ofFill();
         float codeY = mCodeEntry->GetPosition(true).y;
         float topY = ofClamp(linePos.y + 3, codeY, codeY + mCodeEntry->GetRect().height);
         float bottomY = ofClamp(linePos.y + 3 + mCodeEntry->GetCharHeight(), codeY, codeY + mCodeEntry->GetRect().height);
         ofRectangle lineRect(linePos.x, topY, mCodeEntry->GetRect().width, bottomY - topY);
         ofRect(lineRect, L(corner, 0));

         ofSetLineWidth(2);
         ofSetColor(IDrawableModule::GetColor(kModuleCategory_Other), 30);
         float startX, startY, endX, endY;
         ofRectangle targetRect = mBoundModuleConnections[i].mTarget->GetRect();
         FindClosestSides(lineRect.x, lineRect.y, lineRect.width, lineRect.height, targetRect.x - mX, targetRect.y - mY, targetRect.width, targetRect.height, startX, startY, endX, endY, K(sidesOnly));
         ofLine(startX, startY, endX, endY);
      }
   }

   ofPopStyle();
}

void ScriptModule::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource->GetTarget() != nullptr && cableSource->GetConnectionType() == kConnectionType_Note)
      Transport::sDoEventLookahead = true; //scripts that output notes require lookahead to be able to schedule on time
}

bool ScriptModule::MouseMoved(float x, float y)
{
   if (CodeEntry::HasJediNotInstalledWarning())
   {
      ofRectangle buttonRect = mShowReferenceButton->GetRect(true);
      float warningX = buttonRect.getMaxX() + 10;
      float warningY = buttonRect.getCenter().y;
      if (ofDistSquared(x, y, warningX, warningY) <= 6 * 6)
         mShowJediWarning = true;
      else
         mShowJediWarning = false;
   }

   return IDrawableModule::MouseMoved(x, y);
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
   if (sHasPythonEverSuccessfullyInitialized)
      InitializePythonIfNecessary();

   if (!sPythonInitialized)
      return;

   if (ScriptModule::sHasLoadedUntrustedScript)
   {
      if (TheSynth->FindModule("scriptwarning") == nullptr)
      {
         ModuleFactory::Spawnable spawnable;
         spawnable.mLabel = "scriptwarning";
         TheSynth->SpawnModuleOnTheFly(spawnable, 50, 100, true);
         TheSynth->SetAudioPaused(true);
      }
      mCodeEntry->Publish();
      return;
   }

   if (sScriptsRequestingInitExecution.size() > 0)
   {
      for (auto s : sScriptsRequestingInitExecution)
      {
         s->mCodeEntry->Publish();
         s->ExecuteCode();
      }
      sScriptsRequestingInitExecution.clear();
   }

   double time = gTime;

   for (size_t i = 0; i < mScheduledPulseTimes.size(); ++i)
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

   for (size_t i = 0; i < mPendingNoteInput.size(); ++i)
   {
      if (mPendingNoteInput[i].time != -1 &&
          time + TheTransport->GetEventLookaheadMs() > mPendingNoteInput[i].time)
      {
         if (mLastError == "")
         {
            //if (mPendingNoteInput[i].time < time)
            //   ofLog() << "trying to run script triggered by note too late!";
            RunCode(mPendingNoteInput[i].time, "on_note(" + ofToString(mPendingNoteInput[i].pitch) + ", " + ofToString(mPendingNoteInput[i].velocity) + ")");
         }
         mPendingNoteInput[i].time = -1;
      }
   }

   for (size_t i = 0; i < mScheduledUIControlValue.size(); ++i)
   {
      if (mScheduledUIControlValue[i].time != -1 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledUIControlValue[i].time)
      {
         AdjustUIControl(mScheduledUIControlValue[i].control, mScheduledUIControlValue[i].value, mScheduledUIControlValue[i].time, mScheduledUIControlValue[i].lineNum);
         mScheduledUIControlValue[i].time = -1;
      }
   }

   //note offs first
   for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          mScheduledNoteOutput[i].velocity == 0 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledNoteOutput[i].time)
      {
         PlayNote(mScheduledNoteOutput[i].time, mScheduledNoteOutput[i].pitch, mScheduledNoteOutput[i].velocity, mScheduledNoteOutput[i].pan, mScheduledNoteOutput[i].noteOutputIndex, mScheduledNoteOutput[i].lineNum);
         mScheduledNoteOutput[i].time = -1;
      }
   }

   //then note ons
   for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          mScheduledNoteOutput[i].velocity != 0 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledNoteOutput[i].time)
      {
         PlayNote(mScheduledNoteOutput[i].time, mScheduledNoteOutput[i].pitch, mScheduledNoteOutput[i].velocity, mScheduledNoteOutput[i].pan, mScheduledNoteOutput[i].noteOutputIndex, mScheduledNoteOutput[i].lineNum);
         mScheduledNoteOutput[i].time = -1;
      }
   }

   for (size_t i = 0; i < mScheduledMethodCall.size(); ++i)
   {
      if (mScheduledMethodCall[i].time != -1 &&
          time + TheTransport->GetEventLookaheadMs() > mScheduledMethodCall[i].time)
      {
         RunCode(mScheduledMethodCall[i].time, mScheduledMethodCall[i].method);
         mMethodCallTracker.AddEvent(mScheduledMethodCall[i].lineNum);
         mScheduledMethodCall[i].time = -1;
      }
   }

   if (mMidiMessageQueue.size() > 0)
   {
      mMidiMessageQueueMutex.lock();
      for (std::string& methodCall : mMidiMessageQueue)
         RunCode(gTime, methodCall);
      mMidiMessageQueue.clear();
      mMidiMessageQueueMutex.unlock();
   }

   if (mHotloadScripts && !mLoadedScriptPath.empty())
   {
      File scriptFile = File(mLoadedScriptPath);
      if (mLoadedScriptFiletime < scriptFile.getLastModificationTime())
      {
         std::unique_ptr<FileInputStream> input(scriptFile.createInputStream());
         std::string text = input->readString().toStdString();
         ofStringReplace(text, "\r", "");
         mCodeEntry->SetText(text);
         mCodeEntry->Publish();
         ExecuteCode();
         mLoadedScriptFiletime = scriptFile.getLastModificationTime();
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

double ScriptModule::GetScheduledTime(double delayMeasureTime)
{
   return sMostRecentRunTime + delayMeasureTime * TheTransport->MsPerBar();
}

void ScriptModule::PlayNoteFromScript(float pitch, float velocity, float pan, int noteOutputIndex)
{
   PlayNote(sMostRecentRunTime, pitch, velocity, pan, noteOutputIndex, mNextLineToExecute);
}

void ScriptModule::PlayNoteFromScriptAfterDelay(float pitch, float velocity, double delayMeasureTime, float pan, int noteOutputIndex)
{
   double time = GetScheduledTime(delayMeasureTime);
   //if (velocity == 0)
   //   time -= gBufferSizeMs + 1;  //TODO(Ryan) hack to make note offs happen a buffer early... figure out why scheduled lengths are longer than it takes to get the next pulse of the same interval

   //ofLog() << "ScriptModule::PlayNoteFromScriptAfterDelay() " << velocity << " " << time << " " << sMostRecentRunTime << " " << (time - sMostRecentRunTime);

   if (time <= sMostRecentRunTime)
   {
      if (time < gTime)
         ofLog() << "script is trying to play a note in the past!";
      PlayNote(time, pitch, velocity, pan, noteOutputIndex, mNextLineToExecute);
   }
   else
   {
      ScheduleNote(time, pitch, velocity, pan, noteOutputIndex);
   }
}

void ScriptModule::SendCCFromScript(int control, int value, int noteOutputIndex)
{
   if (noteOutputIndex == 0)
   {
      SendCC(control, value);
      return;
   }

   if (noteOutputIndex - 1 < (int)mExtraNoteOutputs.size())
   {
      mExtraNoteOutputs[noteOutputIndex - 1]->SendCCOutput(control, value);
   }
}

void ScriptModule::ScheduleNote(double time, float pitch, float velocity, float pan, int noteOutputIndex)
{
   for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time == -1)
      {
         mScheduledNoteOutput[i].time = time;
         mScheduledNoteOutput[i].startTime = sMostRecentRunTime;
         mScheduledNoteOutput[i].pitch = pitch;
         mScheduledNoteOutput[i].velocity = velocity;
         mScheduledNoteOutput[i].pan = pan;
         mScheduledNoteOutput[i].noteOutputIndex = noteOutputIndex;
         mScheduledNoteOutput[i].lineNum = mNextLineToExecute;
         break;
      }
   }
}

void ScriptModule::ScheduleMethod(std::string method, double delayMeasureTime)
{
   for (size_t i = 0; i < mScheduledMethodCall.size(); ++i)
   {
      if (mScheduledMethodCall[i].time == -1)
      {
         double time = GetScheduledTime(delayMeasureTime);

         mScheduledMethodCall[i].time = time;
         mScheduledMethodCall[i].startTime = sMostRecentRunTime;
         mScheduledMethodCall[i].method = method;
         mScheduledMethodCall[i].lineNum = mNextLineToExecute;
         break;
      }
   }
}

void ScriptModule::ScheduleUIControlValue(IUIControl* control, float value, double delayMeasureTime)
{
   for (size_t i = 0; i < mScheduledUIControlValue.size(); ++i)
   {
      if (mScheduledUIControlValue[i].time == -1)
      {
         double time = GetScheduledTime(delayMeasureTime);

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
   ScriptModule* module = sScriptModules[scriptModuleIndex];
   if (module != sMostRecentLineExecutedModule || sPriorExecutedModule == nullptr)
   {
      sPriorExecutedModule = sMostRecentLineExecutedModule;
      sMostRecentLineExecutedModule = module;
   }
   sScriptModules[scriptModuleIndex]->mNextLineToExecute = lineNum;
   sScriptModules[scriptModuleIndex]->mLineExecuteTracker.AddEvent(lineNum);
}

void ScriptModule::PrintText(std::string text)
{
   for (size_t i = 0; i < mPrintDisplay.size(); ++i)
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

IUIControl* ScriptModule::GetUIControl(std::string path)
{
   if (path == "")
      return nullptr;

   std::string fullPath = path;

   if (path[0] == '$')
      //absolute path provided by the user
      fullPath = path;
   else if (std::count(path.begin(), path.end(), '~') >= 2)
      //absolute path within a prefab
      fullPath = path;
   else if (ofIsStringInString(Path(), "~") && !ofIsStringInString(path, "~"))
      //in a prefab, referencing a script variable
      fullPath = Path() + "~" + path;
   else if (!ofIsStringInString(Path(), "~") && ofIsStringInString(path, "~"))
      //main screen, referencing a module
      fullPath = path;
   else if (ofIsStringInString(Path(), "~") && ofIsStringInString(path, "~"))
      //in a prefab, referencing module in the current or nested prefab
      fullPath = Path().substr(0, Path().rfind('~') + 1) + path;
   else
      //main screen, referencing a script variable
      fullPath = Path() + "~" + path;

   IUIControl* control = TheSynth->FindUIControl(fullPath);

   return control;
}

void ScriptModule::AdjustUIControl(IUIControl* control, float value, double time, int lineNum)
{
   control->SetValue(value, time);

   mUIControlTracker.AddEvent(lineNum);

   for (size_t i = 0; i < mUIControlModifications.size(); ++i)
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

void ScriptModule::PlayNote(double time, float pitch, float velocity, float pan, int noteOutputIndex, int lineNum)
{
   if (velocity > 0)
   {
      //run through any scheduled note offs for this pitch
      for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
      {
         if (mScheduledNoteOutput[i].velocity == 0 &&
             mScheduledNoteOutput[i].pitch == pitch &&
             mScheduledNoteOutput[i].time != -1 &&
             mScheduledNoteOutput[i].time - 3 <= time)
         {
            PlayNote(MIN(mScheduledNoteOutput[i].time, time), mScheduledNoteOutput[i].pitch, mScheduledNoteOutput[i].velocity, mScheduledNoteOutput[i].pan, mScheduledNoteOutput[i].noteOutputIndex, mScheduledNoteOutput[i].lineNum);
            mScheduledNoteOutput[i].time = -1;
         }
      }
   }

   //ofLog() << "ScriptModule::PlayNote() " << velocity << " " << time;
   int intPitch = int(pitch + .5f);
   ModulationParameters modulation;
   modulation.pan = pan;
   if (pitch - intPitch != 0)
   {
      modulation.pitchBend = &mPitchBends[intPitch];
      modulation.pitchBend->SetValue(pitch - intPitch);
   }
   SendNoteToIndex(noteOutputIndex, NoteMessage(time, intPitch, (int)velocity, -1, modulation));

   if (velocity > 0)
      mNotePlayTracker.AddEvent(lineNum, ofToString(pitch) + " " + ofToString(velocity) + " " + ofToString(pan, 1));
}

void ScriptModule::SendNoteToIndex(int index, NoteMessage note)
{
   if (index == 0)
   {
      PlayNoteOutput(note, true);
      return;
   }

   if (index - 1 < (int)mExtraNoteOutputs.size())
   {
      mExtraNoteOutputs[index - 1]->PlayNoteOutput(note, true);
   }
}

void ScriptModule::SetNumNoteOutputs(int num)
{
   while (num - 1 > (int)mExtraNoteOutputs.size())
   {
      auto noteCable = new AdditionalNoteCable();
      noteCable->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      noteCable->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(-1, 0), PatchCableSource::Side::kLeft);
      AddPatchCableSource(noteCable->GetPatchCableSource());
      noteCable->GetPatchCableSource()->SetManualPosition(0, 30 + 20 * (int)mExtraNoteOutputs.size());
      mExtraNoteOutputs.push_back(noteCable);
   }
}

void ScriptModule::ConnectOscInput(int port)
{
   if (port != mOscInputPort)
   {
      mOscInputPort = port;
      OSCReceiver::disconnect();
      OSCReceiver::connect(port);
   }
}

void ScriptModule::oscMessageReceived(const OSCMessage& msg)
{
   std::string address = msg.getAddressPattern().toString().toStdString();
   std::string messageString = address;

   for (int i = 0; i < msg.size(); ++i)
   {
      if (msg[i].isFloat32())
         messageString += " " + ofToString(msg[i].getFloat32());
      if (msg[i].isInt32())
         messageString += " " + ofToString(msg[i].getInt32());
      if (msg[i].isString())
         messageString += " " + msg[i].getString().toStdString();
   }

   RunCode(gTime, "on_osc(\"" + messageString + "\")");
}

void ScriptModule::SysExReceived(const uint8_t* data, int data_size)
{
   // Avoid code injection by preventing the sysex payload to be interpreted as Python
   // - convert the sysex payload to hex
   // - use bytes.fromhex in Python to parse it
   std::ostringstream ss;
   for (size_t i = 0; i < data_size; i++)
      ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(data[i]);
   mMidiMessageQueueMutex.lock();
   mMidiMessageQueue.push_back("on_sysex(bytes.fromhex('" + ss.str() + "'))");
   mMidiMessageQueueMutex.unlock();
}

void ScriptModule::MidiReceived(MidiMessageType messageType, int control, float value, int channel)
{
   mMidiMessageQueueMutex.lock();
   mMidiMessageQueue.push_back("on_midi(" + ofToString((int)messageType) + ", " + ofToString(control) + ", " + ofToString(value) + ", " + ofToString(channel) + ")");
   mMidiMessageQueueMutex.unlock();
}

void ScriptModule::ButtonClicked(ClickButton* button, double time)
{
   if (button == mPythonInstalledConfirmButton)
      InitializePythonIfNecessary();

   if (button == mRunButton)
   {
      mCodeEntry->Publish();
      RunScript(time);
   }

   if (button == mStopButton)
      Stop();

   if (button == mSaveScriptButton)
   {
      FileChooser chooser("Save script as...", File(ofToDataPath("scripts/script.py")), "*.py", true, false, TheSynth->GetFileChooserParent());
      if (chooser.browseForFileToSave(true))
      {
         std::string path = chooser.getResult().getFullPathName().toStdString();

         File resourceFile(path);
         TemporaryFile tempFile(resourceFile);

         {
            FileOutputStream output(tempFile.getFile());

            if (!output.openedOk())
            {
               DBG("FileOutputStream didn't open correctly ...");
               return;
            }

            output.writeText(mCodeEntry->GetText(false), false, false, nullptr);
            output.flush(); // (called explicitly to force an fsync on posix)

            if (output.getStatus().failed())
            {
               DBG("An error occurred in the FileOutputStream");
               return;
            }
         }

         bool success = tempFile.overwriteTargetFileWithTemporary();
         if (!success)
         {
            DBG("An error occurred writing the file");
            return;
         }

         RefreshScriptFiles();

         for (size_t i = 0; i < mScriptFilePaths.size(); ++i)
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
         mLoadedScriptPath = mScriptFilePaths[mLoadScriptIndex];
         File resourceFile = File(mLoadedScriptPath);

         if (!resourceFile.existsAsFile())
         {
            DBG("File doesn't exist ...");
            return;
         }

         std::unique_ptr<FileInputStream> input(resourceFile.createInputStream());

         if (!input->openedOk())
         {
            DBG("Failed to open file");
            return;
         }

         mLoadedScriptFiletime = resourceFile.getLastModificationTime();

         std::string text = input->readString().toStdString();
         ofStringReplace(text, "\r", "");
         mCodeEntry->SetText(text);
      }
   }

   if (button == mShowReferenceButton)
   {
      float moduleX, moduleY, moduleW, moduleH;
      GetPosition(moduleX, moduleY);
      GetDimensions(moduleW, moduleH);
      ModuleFactory::Spawnable spawnable;
      spawnable.mLabel = "scriptingreference";
      TheSynth->SpawnModuleOnTheFly(spawnable, moduleX + moduleW, moduleY, true);
   }

   if (button == mTrustScriptButton)
   {
      RecordScriptAsTrusted();
      mIsScriptUntrusted = false;
   }

   if (button == mDontTrustScriptButton)
   {
      TheSynth->SetAudioPaused(false);
      TheSynth->ReloadInitialLayout();
   }
}

void ScriptModule::DropdownClicked(DropdownList* list)
{
   if (list == mLoadScriptSelector)
      RefreshScriptFiles();
}

void ScriptModule::DropdownUpdated(DropdownList* list, int oldValue, double time)
{
}

void ScriptModule::RefreshStyleFiles()
{
   ofxJSONElement root;
   if (File(ofToDataPath("scriptstyles.json")).existsAsFile())
      root.open(ofToDataPath("scriptstyles.json"));
   else
      root.open(ofToResourcePath("userdata_original/scriptstyles.json"));
   sStyleJSON = root["styles"];
}

void ScriptModule::RefreshScriptFiles()
{
   mScriptFilePaths.clear();
   mLoadScriptSelector->Clear();
   std::list<std::string> scripts;
   for (const auto& entry : RangedDirectoryIterator{ File{ ofToDataPath("scripts") }, false, "*.py" })
   {
      const auto& file = entry.getFile();
      scripts.push_back(file.getFileName().toStdString());
   }

   scripts.sort();

   for (const auto& script : scripts)
   {
      mLoadScriptSelector->AddLabel(script, (int)mScriptFilePaths.size());
      mScriptFilePaths.push_back(ofToDataPath("scripts/" + script));
   }
}

void ScriptModule::ExecuteCode()
{
   RunScript(NextBufferTime(false));
}

std::pair<int, int> ScriptModule::ExecuteBlock(int lineStart, int lineEnd)
{
   return RunScript(gTime, lineStart, lineEnd);
}

void ScriptModule::OnCodeUpdated()
{
   if (mBoundModuleConnections.size() > 0)
   {
      std::vector<std::string> lines = mCodeEntry->GetLines(false);

      for (size_t i = 0; i < mBoundModuleConnections.size(); ++i)
      {
         if (mBoundModuleConnections[i].mLineText != lines[mBoundModuleConnections[i].mLineIndex])
         {
            bool found = false;
            for (int j = 0; j < (int)lines.size(); ++j)
            {
               if (lines[j] == mBoundModuleConnections[i].mLineText)
               {
                  found = true;
                  mBoundModuleConnections[i].mLineIndex = j;
               }
            }

            if (!found)
               mBoundModuleConnections[i].mTarget = nullptr;
         }
      }
   }
}

void ScriptModule::OnPulse(double time, float velocity, int flags)
{
   for (size_t i = 0; i < mScheduledPulseTimes.size(); ++i)
   {
      if (mScheduledPulseTimes[i] == -1)
      {
         mScheduledPulseTimes[i] = time;
         break;
      }
   }
}

//INoteReceiver
void ScriptModule::PlayNote(NoteMessage note)
{
   for (size_t i = 0; i < mPendingNoteInput.size(); ++i)
   {
      if (mPendingNoteInput[i].time == -1)
      {
         mPendingNoteInput[i].time = note.time;
         mPendingNoteInput[i].pitch = note.pitch;
         mPendingNoteInput[i].velocity = note.velocity;
         break;
      }
   }
}

std::string ScriptModule::GetThisName()
{
   return "me__" + ofToString(mScriptModuleIndex);
}

std::pair<int, int> ScriptModule::RunScript(double time, int lineStart /*=-1*/, int lineEnd /*=-1*/)
{
   //should only be called from main thread

   if (!sPythonInitialized)
   {
      TheSynth->LogEvent("trying to call ScriptModule::RunScript() before python is initialized", kLogEventType_Error);
      return std::make_pair(0, 0);
   }

   py::exec(GetThisName() + " = scriptmodule.get_me(" + ofToString(mScriptModuleIndex) + ")", py::globals());
   std::string code = mCodeEntry->GetText(true);
   std::vector<std::string> lines = ofSplitString(code, "\n");

   int executionStartLine = 0;
   int executionEndLine = (int)lines.size();
   if (lineStart != -1)
   {
      for (auto i = lineStart; i >= 0; --i)
      {
         if (lines[i][0] != ' ') //no indentation
         {
            executionStartLine = i;
            break;
         }
      }

      for (auto i = lineEnd + 1; i < (int)lines.size(); ++i)
      {
         if (lines[i][0] != ' ') //no indentation
         {
            executionEndLine = i - 1;
            break;
         }
      }
   }

   code = "";
   for (size_t i = 0; i < lines.size(); ++i)
   {
      std::string prefix = "";
      if (i < executionStartLine || i > executionEndLine)
         prefix = "#";
      if (ShouldDisplayLineExecutionPre(i > 0 ? lines[i - 1] : "", lines[i]))
         code += prefix + GetIndentation(lines[i]) + "me.highlight_line(" + ofToString(i) + "," + ofToString(mScriptModuleIndex) + ")               ###instrumentation###\n";
      code += prefix + lines[i] + "\n";
   }
   FixUpCode(code);
   mLastRunLiteralCode = code;

   RunCode(time, code);

   return std::make_pair(executionStartLine, executionEndLine);
}

void ScriptModule::RunCode(double time, std::string code)
{
   //should only be called from main thread

   if (!sPythonInitialized)
   {
      TheSynth->LogEvent("trying to call ScriptModule::RunCode() before python is initialized", kLogEventType_Error);
      return;
   }

   if (sHasLoadedUntrustedScript)
   {
      TheSynth->LogEvent("can't run scripts until user has added all loaded scripts to the allow list", kLogEventType_Error);
      return;
   }

   sMostRecentRunTime = time;
   mNextLineToExecute = -1;
   ComputeSliders(0);
   sPriorExecutedModule = nullptr;

   try
   {
      FixUpCode(code);
      py::exec(code, py::globals());

      mCodeEntry->SetError(false);
      mLastError = "";
   }
   catch (pybind11::error_already_set& e)
   {
      ofLog() << "python execution exception (error_already_set): " << e.what();

      if (mNextLineToExecute == -1) //this script hasn't executed yet
         sMostRecentLineExecutedModule = this;

      sMostRecentLineExecutedModule->mLastError = (std::string)py::str(e.type()) + ": " + (std::string)py::str(e.value());

      int lineNumber = sMostRecentLineExecutedModule->mNextLineToExecute;
      if (lineNumber == -1)
      {
         std::string errorString = (std::string)py::str(e.value());
         const std::string lineTextLabel = " line ";
         const char* lineTextPos = strstr(errorString.c_str(), lineTextLabel.c_str());
         if (lineTextPos != nullptr)
         {
            try
            {
               size_t start = lineTextPos + lineTextLabel.length() - errorString.c_str();
               size_t len = errorString.size() - 1 - start;
               std::string lineNumberText = errorString.substr(start, len);
               int rawLineNumber = stoi(lineNumberText);
               int realLineNumber = rawLineNumber - 1;

               std::vector<std::string> lines = ofSplitString(sMostRecentLineExecutedModule->mLastRunLiteralCode, "\n");
               for (size_t i = 0; i < lines.size() && i < rawLineNumber; ++i)
               {
                  if (ofIsStringInString(lines[i], "###instrumentation###"))
                     --realLineNumber;
               }

               lineNumber = realLineNumber;
            }
            catch (std::exception)
            {
            }
         }
      }

      sMostRecentLineExecutedModule->mCodeEntry->SetError(true, lineNumber);
   }
   catch (const std::exception& e)
   {
      ofLog() << "python execution exception: " << e.what();
   }
}

std::string ScriptModule::GetMethodPrefix()
{
   std::string prefix = Path();
   ofStringReplace(prefix, "~", "");
   return prefix;
}

void ScriptModule::FixUpCode(std::string& code)
{
   std::string prefix = GetMethodPrefix();
   ofStringReplace(code, "on_pulse(", "on_pulse__" + prefix + "(");
   ofStringReplace(code, "on_note(", "on_note__" + prefix + "(");
   ofStringReplace(code, "on_grid_button(", "on_grid_button__" + prefix + "(");
   ofStringReplace(code, "on_osc(", "on_osc__" + prefix + "(");
   ofStringReplace(code, "on_midi(", "on_midi__" + prefix + "(");
   ofStringReplace(code, "on_sysex(", "on_sysex__" + prefix + "(");
   ofStringReplace(code, "this.", GetThisName() + ".");
   ofStringReplace(code, "me.", GetThisName() + ".");
}

void ScriptModule::GetFirstAndLastCharacter(std::string line, char& first, char& last)
{
   bool hasFirstCharacter = false;
   first = 0;
   last = 0;
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

bool ScriptModule::ShouldDisplayLineExecutionPre(std::string priorLine, std::string line)
{
   if (!IsNonWhitespace(line))
      return false;

   char firstCharacter;
   char lastCharacter;

   GetFirstAndLastCharacter(priorLine, firstCharacter, lastCharacter);
   if (firstCharacter == '@')
      return false;
   if (lastCharacter == ',')
      return false;

   GetFirstAndLastCharacter(line, firstCharacter, lastCharacter);
   if (firstCharacter == '@')
      return false;
   if (firstCharacter == '#')
      return false;
   if (lastCharacter == ':' && (ofIsStringInString(line, "else") || ofIsStringInString(line, "elif") || ofIsStringInString(line, "except")))
      return false;
   return true;
}

std::string ScriptModule::GetIndentation(std::string line)
{
   std::string ret;
   for (size_t i = 0; i < line.length(); ++i)
   {
      if (line[i] == ' ')
         ret += ' ';
      else
         break;
   }
   return ret;
}

bool ScriptModule::IsNonWhitespace(std::string line)
{
   for (size_t i = 0; i < line.length(); ++i)
   {
      if (line[i] != ' ')
         return true;
   }
   return false;
}

static std::string sContextToRestore = "";
void ScriptModule::SetContext()
{
   sContextToRestore = IClickable::sPathLoadContext;
   if (GetOwningContainer()->GetOwner() != nullptr)
      IClickable::SetLoadContext(GetOwningContainer()->GetOwner());
}

void ScriptModule::ClearContext()
{
   IClickable::sPathLoadContext = sContextToRestore;
}

void ScriptModule::OnModuleReferenceBound(IDrawableModule* target)
{
   if (target != nullptr)
   {
      for (size_t i = 0; i < mBoundModuleConnections.size(); ++i)
      {
         if (mBoundModuleConnections[i].mTarget == target)
            return;
      }

      std::string code = mCodeEntry->GetText(true);
      std::vector<std::string> lines = ofSplitString(code, "\n");
      if (mNextLineToExecute >= 0 && mNextLineToExecute < lines.size())
      {
         BoundModuleConnection connection;
         connection.mLineIndex = mNextLineToExecute;
         connection.mLineText = lines[mNextLineToExecute];
         connection.mTarget = target;
         mBoundModuleConnections.push_back(connection);
      }
   }
}

void ScriptModule::Stop()
{
   double time = NextBufferTime(false);

   //run through any scheduled note offs for this pitch
   for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
   {
      if (mScheduledNoteOutput[i].time != -1 &&
          mScheduledNoteOutput[i].velocity == 0)
      {
         PlayNote(time, mScheduledNoteOutput[i].pitch, 0, 0, mScheduledNoteOutput[i].noteOutputIndex, mScheduledNoteOutput[i].lineNum);
         mScheduledNoteOutput[i].time = -1;
      }
   }

   Reset();
}

void ScriptModule::Reset()
{
   for (size_t i = 0; i < mScheduledPulseTimes.size(); ++i)
      mScheduledPulseTimes[i] = -1;

   for (size_t i = 0; i < mScheduledNoteOutput.size(); ++i)
      mScheduledNoteOutput[i].time = -1;

   for (size_t i = 0; i < mScheduledMethodCall.size(); ++i)
      mScheduledMethodCall[i].time = -1;

   for (size_t i = 0; i < mScheduledUIControlValue.size(); ++i)
      mScheduledUIControlValue[i].time = -1;

   for (size_t i = 0; i < mPendingNoteInput.size(); ++i)
      mPendingNoteInput[i].time = -1;

   for (size_t i = 0; i < mPrintDisplay.size(); ++i)
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

void ScriptModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("execute_on_init", moduleInfo, true);
   mModuleSaveData.LoadInt("init_execute_priority", moduleInfo, 0, -9999, 9999, K(isTextField));
   mModuleSaveData.LoadBool("syntax_highlighting", moduleInfo, true);
   mModuleSaveData.LoadString("style", moduleInfo, "classic", [](DropdownList* list)
                              {
                                 for (auto i = 0; i < sStyleJSON.size(); ++i)
                                 {
                                    try
                                    {
                                       list->AddLabel(sStyleJSON[i]["name"].asString(), i);
                                    }
                                    catch (Json::LogicError& e)
                                    {
                                       TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
                                    }
                                 }
                              });
   mModuleSaveData.LoadBool("hotload_script_files", moduleInfo, false);
   mModuleSaveData.LoadBool("draw_bound_module_connections", moduleInfo, true);

   SetUpFromSaveData();
}

void ScriptModule::SetUpFromSaveData()
{
   if (mModuleSaveData.GetBool("execute_on_init"))
   {
      mInitExecutePriority = mModuleSaveData.GetInt("init_execute_priority");
      sScriptsRequestingInitExecution.insert(std::lower_bound(sScriptsRequestingInitExecution.begin(),
                                                              sScriptsRequestingInitExecution.end(),
                                                              this,
                                                              [](const ScriptModule* left, const ScriptModule* right)
                                                              {
                                                                 return left->mInitExecutePriority < right->mInitExecutePriority;
                                                              }),
                                             this);
   }

   mCodeEntry->SetDoSyntaxHighlighting(mModuleSaveData.GetBool("syntax_highlighting"));

   std::string styleName = mModuleSaveData.GetString("style");
   for (auto i = 0; i < sStyleJSON.size(); ++i)
   {
      try
      {
         if (sStyleJSON[i]["name"].asString() == styleName)
            mCodeEntry->SetStyleFromJSON(sStyleJSON[i]);
      }
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }

   mHotloadScripts = mModuleSaveData.GetBool("hotload_script_files");
   mDrawBoundModuleConnections = mModuleSaveData.GetBool("draw_bound_module_connections");
}

void ScriptModule::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ScriptModule::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   out << (int)mExtraNoteOutputs.size();

   IDrawableModule::SaveState(out);

   out << mWidth;
   out << mHeight;

   RecordScriptAsTrusted();
}

void ScriptModule::LoadState(FileStreamIn& in, int rev)
{
   if (ModularSynth::sLoadingFileSaveStateRev >= 421 && ModularSynth::sLoadingFileSaveStateRev < 423)
   {
      in >> rev;
      LoadStateValidate(rev <= GetModuleSaveStateRev());
   }

   if (rev >= 2)
   {
      int extraNoteOutputs;
      in >> extraNoteOutputs;
      SetNumNoteOutputs(extraNoteOutputs + 1);
   }

   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev == 420)
   {
      in >> rev;
      LoadStateValidate(rev <= GetModuleSaveStateRev());
   }

   float w, h;
   in >> w;
   in >> h;
   Resize(w, h);

   juce::String checksum = GetScriptChecksum();
   juce::File trusted_python_scripts = File(ofToDataPath("internal/trusted_python_scripts"));
   bool isScriptTrusted = false;
   if (trusted_python_scripts.existsAsFile())
   {
      StringArray lines;
      trusted_python_scripts.readLines(lines);
      for (auto& line : lines)
      {
         if (line == checksum)
            isScriptTrusted = true;
      }
   }

   if (!isScriptTrusted)
      sHasLoadedUntrustedScript = true;
   mIsScriptUntrusted = !isScriptTrusted;
}

juce::String ScriptModule::GetScriptChecksum() const
{
   juce::String code = mCodeEntry->GetText(false);
   juce::String checksum = juce::SHA256(code.toUTF8()).toHexString();
   return checksum;
}

void ScriptModule::RecordScriptAsTrusted()
{
   juce::File trusted_python_scripts = File(ofToDataPath("internal/trusted_python_scripts"));
   if (!trusted_python_scripts.existsAsFile())
      trusted_python_scripts.create();
   trusted_python_scripts.appendText("\n" + GetScriptChecksum());
}

void ScriptModule::LineEventTracker::Draw(CodeEntry* codeEntry, int style, ofColor color)
{
   ofPushStyle();
   ofFill();
   for (int i = 0; i < (int)mText.size(); ++i)
   {
      float alpha = style == 0 ? 200 : 150;
      float fadeMs = style == 0 ? 200 : 150;
      if (gTime - mTimes[i] > 0 && gTime - mTimes[i] < fadeMs)
      {
         ofSetColor(color, alpha * (1 - (gTime - mTimes[i]) / fadeMs));
         ofVec2f linePos = codeEntry->GetLinePos(i, false);
         if (style == 0)
            ofRect(linePos.x + 1, linePos.y + 3, 4, codeEntry->GetCharHeight(), L(corner, 0));
         if (style == 1)
            ofCircle(linePos.x + 11, linePos.y + 10, 5);

         if (mText[i] != "")
         {
            linePos = codeEntry->GetLinePos(i, K(end));
            DrawTextNormal(mText[i], linePos.x + 10, linePos.y + 15);
         }
      }
   }
   ofPopStyle();
}

ScriptReferenceDisplay::ScriptReferenceDisplay()
{
   LoadText();
}

void ScriptReferenceDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mCloseButton = new ClickButton(this, "close", 3, 3);
}

ScriptReferenceDisplay::~ScriptReferenceDisplay()
{
}

void ScriptReferenceDisplay::LoadText()
{
   File file(ofToResourcePath("scripting_reference.txt").c_str());
   if (file.existsAsFile())
   {
      std::string text = file.loadFileAsString().toStdString();
      ofStringReplace(text, "\r", "");
      mText = ofSplitString(text, "\n");
   }

   mMaxScrollAmount = (int)mText.size() * 14;
}

void ScriptReferenceDisplay::DrawModule()
{
   mCloseButton->Draw();

   float y = 34;
   for (size_t i = 0; i < mText.size(); ++i)
   {
      DrawTextNormal(mText[i], 4 - mScrollOffset.x, y - mScrollOffset.y);
      y += 14;
   }
}

bool ScriptReferenceDisplay::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mScrollOffset.y = ofClamp(mScrollOffset.y - scrollY * 10, 0, mMaxScrollAmount);
   return true;
}

void ScriptReferenceDisplay::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

void ScriptReferenceDisplay::ButtonClicked(ClickButton* button, double time)
{
   if (button == mCloseButton)
      GetOwningContainer()->DeleteModule(this);
}

void ScriptWarningPopup::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void ScriptWarningPopup::Poll()
{
   IDrawableModule::Poll();

   std::vector<IDrawableModule*> modules;
   TheSynth->GetAllModules(modules);

   int untrustedCount = 0;
   for (auto* module : modules)
   {
      ScriptModule* script = dynamic_cast<ScriptModule*>(module);
      if (script != nullptr && !script->IsScriptTrusted())
         ++untrustedCount;
   }
   mRemainingUntrustedScriptModules = untrustedCount;

   if (untrustedCount == 0)
   {
      TheSynth->SetAudioPaused(false);
      ScriptModule::sHasLoadedUntrustedScript = false;
      if (Prefab::sLastLoadWasPrefab)
      {
         //get rid of this popup and continue loading prefab
         GetOwningContainer()->DeleteModule(this);
      }
      else
      {
         //reload save file fresh
         TheSynth->LoadState(TheSynth->GetLastSavePath());
      }
   }
}

void ScriptWarningPopup::DrawModule()
{
   DrawTextNormal(std::string("") +
                  "heads up: this BSK file contains python scripts.\n\n" +
                  "generally, these python scripts are innocuous, and used to do cool things within bespoke.\n" +
                  "however, a malicious user could theoretically use python scripts to do bad things to your computer.\n\n" +
                  "do you trust the creator of this file? if so, please go to each script window and approve the script\n" +
                  "(" + ofToString(mRemainingUntrustedScriptModules) + " script" + (mRemainingUntrustedScriptModules == 1 ? "" : "s") + " remaining to be approved)",
                  3, 14);
}
