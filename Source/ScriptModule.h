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

    ScriptModule.h
    Created: 19 Apr 2020 1:52:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "CodeEntry.h"
#include "ClickButton.h"
#include "NoteEffectBase.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ModulationChain.h"
#include "MidiController.h"

#include "juce_osc/juce_osc.h"

class ScriptModule : public IDrawableModule, public IButtonListener, public NoteEffectBase, public IPulseReceiver, public ICodeEntryListener, public IFloatSliderListener, public IDropdownListener, private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
   ScriptModule();
   virtual ~ScriptModule();
   static IDrawableModule* Create() { return new ScriptModule(); }

   static void UninitializePython();
   static void InitializePythonIfNecessary();
   static void CheckIfPythonEverSuccessfullyInitialized();


   void CreateUIControls() override;

   void Poll() override;

   void PlayNoteFromScript(float pitch, float velocity, float pan, int noteOutputIndex);
   void PlayNoteFromScriptAfterDelay(float pitch, float velocity, double delayMeasureTime, float pan, int noteOutputIndex);
   void SendCCFromScript(int control, int value, int noteOutputIndex);
   void ScheduleMethod(std::string method, double delayMeasureTime);
   void ScheduleUIControlValue(IUIControl* control, float value, double delayMeasureTime);
   void HighlightLine(int lineNum, int scriptModuleIndex);
   void PrintText(std::string text);
   IUIControl* GetUIControl(std::string path);
   void Stop();
   double GetScheduledTime(double delayMeasureTime);
   void SetNumNoteOutputs(int num);
   void ConnectOscInput(int port);
   void MidiReceived(MidiMessageType messageType, int control, float value, int channel);
   void OnModuleReferenceBound(IDrawableModule* target);
   void SetContext();
   void ClearContext();

   void RunCode(double time, std::string code);

   void OnPulse(double time, float velocity, int flags) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldValue) override {}
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldValue) override;

   //ICodeEntryListener
   void ExecuteCode() override;
   std::pair<int, int> ExecuteBlock(int lineStart, int lineEnd) override;
   void OnCodeUpdated() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //OSCReceiver
   void oscMessageReceived(const juce::OSCMessage& msg) override;

   bool HasDebugDraw() const override { return true; }

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;

   static std::vector<ScriptModule*> sScriptModules;
   static std::list<ScriptModule*> sScriptsRequestingInitExecution;
   static ScriptModule* sMostRecentLineExecutedModule;
   static ScriptModule* sPriorExecutedModule;
   static float GetScriptMeasureTime();
   static float GetTimeSigRatio();
   static std::string sBackgroundTextString;
   static float sBackgroundTextSize;
   static ofVec2f sBackgroundTextPos;
   static ofColor sBackgroundTextColor;
   static bool sPythonInitialized;
   static bool sHasPythonEverSuccessfullyInitialized;

   ModulationChain* GetPitchBend(int pitch) { return &mPitchBends[pitch]; }
   ModulationChain* GetModWheel(int pitch) { return &mModWheels[pitch]; }
   ModulationChain* GetPressure(int pitch) { return &mPressures[pitch]; }

   static std::string GetBootstrapImportString() { return "import bespoke; import module; import scriptmodule; import random; import math"; }

private:
   void PlayNote(double time, float pitch, float velocity, float pan, int noteOutputIndex, int lineNum);
   void AdjustUIControl(IUIControl* control, float value, int lineNum);
   std::pair<int, int> RunScript(double time, int lineStart = -1, int lineEnd = -1);
   void FixUpCode(std::string& code);
   void ScheduleNote(double time, float pitch, float velocity, float pan, int noteOutputIndex);
   void SendNoteToIndex(int index, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation);
   std::string GetThisName();
   std::string GetIndentation(std::string line);
   std::string GetMethodPrefix();
   bool ShouldDisplayLineExecutionPre(std::string priorLine, std::string line);
   void GetFirstAndLastCharacter(std::string line, char& first, char& last);
   bool IsNonWhitespace(std::string line);
   void DrawTimer(int lineNum, double startTime, double endTime, ofColor color, bool filled);
   void RefreshScriptFiles();
   void RefreshStyleFiles();
   void Reset();

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   ClickButton* mPythonInstalledConfirmButton{ nullptr };
   DropdownList* mLoadScriptSelector{ nullptr };
   ClickButton* mLoadScriptButton{ nullptr };
   ClickButton* mSaveScriptButton{ nullptr };
   ClickButton* mShowReferenceButton{ nullptr };
   CodeEntry* mCodeEntry{ nullptr };
   ClickButton* mRunButton{ nullptr };
   ClickButton* mStopButton{ nullptr };
   FloatSlider* mASlider{ nullptr };
   FloatSlider* mBSlider{ nullptr };
   FloatSlider* mCSlider{ nullptr };
   FloatSlider* mDSlider{ nullptr };
   int mLoadScriptIndex{ 0 };
   std::string mLoadedScriptPath;
   juce::Time mLoadedScriptFiletime;
   bool mHotloadScripts{ false };
   static ofxJSONElement sStyleJSON;
   float mA{ 0 };
   float mB{ 0 };
   float mC{ 0 };
   float mD{ 0 };

   float mWidth{ 200 };
   float mHeight{ 20 };
   std::array<double, 20> mScheduledPulseTimes{};
   static double sMostRecentRunTime;
   std::string mLastError;
   size_t mScriptModuleIndex;
   std::string mLastRunLiteralCode;
   int mNextLineToExecute{ -1 };
   int mInitExecutePriority{ 0 };
   int mOscInputPort{ -1 };

   struct ScheduledNoteOutput
   {
      double startTime{ 0 };
      double time{ 0 };
      float pitch{ 0 };
      float velocity{ 0 };
      float pan{ .5 };
      int noteOutputIndex{ -1 };
      int lineNum{ -1 };
   };
   std::array<ScheduledNoteOutput, 200> mScheduledNoteOutput;

   struct ScheduledMethodCall
   {
      double startTime{ 0 };
      double time{ 0 };
      std::string method;
      int lineNum{ -1 };
   };
   std::array<ScheduledMethodCall, 50> mScheduledMethodCall;

   struct ScheduledUIControlValue
   {
      double startTime{ 0 };
      double time{ 0 };
      IUIControl* control{ nullptr };
      float value{ 0 };
      int lineNum{ -1 };
   };
   std::array<ScheduledUIControlValue, 50> mScheduledUIControlValue;

   struct PendingNoteInput
   {
      double time{ 0 };
      int pitch{ 0 };
      int velocity{ 0 };
   };
   std::array<PendingNoteInput, 50> mPendingNoteInput;

   struct PrintDisplay
   {
      double time{ 0 };
      std::string text;
      int lineNum{ -1 };
   };
   std::array<PrintDisplay, 10> mPrintDisplay;

   struct UIControlModificationDisplay
   {
      double time{ 0 };
      ofVec2f position;
      float value{ 0 };
      int lineNum{ -1 };
   };
   std::array<UIControlModificationDisplay, 10> mUIControlModifications;

   class LineEventTracker
   {
   public:
      LineEventTracker()
      {
         for (size_t i = 0; i < mTimes.size(); ++i)
            mTimes[i] = -999;
      }

      void AddEvent(int lineNum, std::string text = "")
      {
         if (lineNum >= 0 && lineNum < (int)mTimes.size())
         {
            mTimes[lineNum] = gTime;
            mText[lineNum] = text;
         }
      }

      void Draw(CodeEntry* codeEntry, int style, ofColor color);

   private:
      std::array<double, 256> mTimes{};
      std::array<std::string, 256> mText{};
   };

   LineEventTracker mLineExecuteTracker;
   LineEventTracker mMethodCallTracker;
   LineEventTracker mNotePlayTracker;
   LineEventTracker mUIControlTracker;

   struct BoundModuleConnection
   {
      int mLineIndex{ -1 };
      std::string mLineText;
      IDrawableModule* mTarget{ nullptr };
   };
   std::vector<BoundModuleConnection> mBoundModuleConnections;

   std::vector<std::string> mScriptFilePaths;

   std::vector<AdditionalNoteCable*> mExtraNoteOutputs{};
   std::array<ModulationChain, 128> mPitchBends{};
   std::array<ModulationChain, 128> mModWheels{};
   std::array<ModulationChain, 128> mPressures{};
   std::list<std::string> mMidiMessageQueue;
   ofMutex mMidiMessageQueueMutex;

   bool mShowJediWarning{ false };
};

class ScriptReferenceDisplay : public IDrawableModule, public IButtonListener
{
public:
   ScriptReferenceDisplay();
   virtual ~ScriptReferenceDisplay();
   static IDrawableModule* Create() { return new ScriptReferenceDisplay(); }


   void CreateUIControls() override;

   void ButtonClicked(ClickButton* button) override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = w;
      mHeight = h;
   }
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;

   void LoadText();

   std::vector<std::string> mText;
   ClickButton* mCloseButton{ nullptr };
   float mWidth{ 750 };
   float mHeight{ 335 };
   ofVec2f mScrollOffset;
   float mMaxScrollAmount{ 0 };
};
