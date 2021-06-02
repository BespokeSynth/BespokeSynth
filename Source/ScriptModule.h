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

class ScriptModule : public IDrawableModule, public IButtonListener, public NoteEffectBase, public IPulseReceiver, public ICodeEntryListener, public IFloatSliderListener, public IDropdownListener,
                     private OSCReceiver,
                     private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
public:
   ScriptModule();
   virtual ~ScriptModule();
   static IDrawableModule* Create() { return new ScriptModule(); }
   
   static void UninitializePython();
   static void InitializePythonIfNecessary();
   
   string GetTitleLabel() override { return "script"; }
   void CreateUIControls() override;
   
   void Poll() override;
   
   void PlayNoteFromScript(float pitch, float velocity, float pan, int noteOutputIndex);
   void PlayNoteFromScriptAfterDelay(float pitch, float velocity, double delayMeasureTime, float pan, int noteOutputIndex);
   void ScheduleMethod(string method, double delayMeasureTime);
   void ScheduleUIControlValue(IUIControl* control, float value, double delayMeasureTime);
   void HighlightLine(int lineNum, int scriptModuleIndex);
   void PrintText(string text);
   IUIControl* GetUIControl(string path);
   void Stop();
   double GetScheduledTime(double delayMeasureTime);
   void SetNumNoteOutputs(int num);
   void ConnectOscInput(int port);
   void MidiReceived(MidiMessageType messageType, int control, float value, int channel);
   void OnModuleReferenceBound(IDrawableModule* target);
   
   void RunCode(double time, string code);
   
   void OnPulse(double time, float velocity, int flags) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldValue) override {}
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldValue) override {}
 
   //ICodeEntryListener
   void ExecuteCode() override;
   pair<int,int> ExecuteBlock(int lineStart, int lineEnd) override;
   void OnCodeUpdated() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //OSCReceiver
   void oscMessageReceived(const OSCMessage& msg) override;
   
   bool HasDebugDraw() const override { return true; }
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;

   static std::vector<ScriptModule*> sScriptModules;
   static std::list<ScriptModule*> sScriptsRequestingInitExecution;
   static ScriptModule* sMostRecentLineExecutedModule;
   static ScriptModule* sPriorExecutedModule;
   static float GetScriptMeasureTime();
   static float GetTimeSigRatio();
   static string sBackgroundTextString;
   static float sBackgroundTextSize;
   static ofColor sBackgroundTextColor;

   ModulationChain* GetPitchBend(int pitch) { return &mPitchBends[pitch]; }
   ModulationChain* GetModWheel(int pitch) { return &mModWheels[pitch]; }
   ModulationChain* GetPressure(int pitch) { return &mPressures[pitch]; }

   static string GetBootstrapImportString() { return "import bespoke; import module; import scriptmodule; import random; import math"; }
   
private:
   void PlayNote(double time, float pitch, float velocity, float pan, int noteOutputIndex, int lineNum);
   void AdjustUIControl(IUIControl* control, float value, int lineNum);
   pair<int,int> RunScript(double time, int lineStart = -1, int lineEnd = -1);
   void FixUpCode(string& code);
   void ScheduleNote(double time, float pitch, float velocity, float pan, int noteOutputIndex);
   void SendNoteToIndex(int index, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation);
   string GetThisName();
   string GetIndentation(string line);
   string GetMethodPrefix();
   bool ShouldDisplayLineExecutionPre(string priorLine, string line);
   void GetFirstAndLastCharacter(string line, char& first, char& last);
   bool IsNonWhitespace(string line);
   void DrawTimer(int lineNum, double startTime, double endTime, ofColor color, bool filled);
   void RefreshScriptFiles();
   void Reset();

   static void CheckIfPythonEverSuccessfullyInitialized();
   
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   
   DropdownList* mLoadScriptSelector;
   ClickButton* mLoadScriptButton;
   ClickButton* mSaveScriptButton;
   CodeEntry* mCodeEntry;
   ClickButton* mRunButton;
   ClickButton* mStopButton;
   FloatSlider* mASlider;
   FloatSlider* mBSlider;
   FloatSlider* mCSlider;
   FloatSlider* mDSlider;
   int mLoadScriptIndex;
   float mA;
   float mB;
   float mC;
   float mD;
   
   float mWidth;
   float mHeight;
   std::array<double, 20> mScheduledPulseTimes;
   static double sMostRecentRunTime;
   string mLastError;
   size_t mScriptModuleIndex;
   string mLastRunLiteralCode;
   int mNextLineToExecute;
   int mInitExecutePriority;
   int mOscInputPort;
   
   struct ScheduledNoteOutput
   {
      double startTime;
      double time;
      float pitch;
      float velocity;
      float pan;
      int noteOutputIndex;
      int lineNum;
   };
   std::array<ScheduledNoteOutput, 200> mScheduledNoteOutput;
   
   struct ScheduledMethodCall
   {
      double startTime;
      double time;
      string method;
      int lineNum;
   };
   std::array<ScheduledMethodCall, 50> mScheduledMethodCall;
   
   struct ScheduledUIControlValue
   {
      double startTime;
      double time;
      IUIControl* control;
      float value;
      int lineNum;
   };
   std::array<ScheduledUIControlValue, 50> mScheduledUIControlValue;
   
   struct PendingNoteInput
   {
      double time;
      int pitch;
      int velocity;
   };
   std::array<PendingNoteInput, 50> mPendingNoteInput;
   
   struct PrintDisplay
   {
      double time;
      string text;
      int lineNum;
   };
   std::array<PrintDisplay, 10> mPrintDisplay;
   
   struct UIControlModificationDisplay
   {
      double time;
      ofVec2f position;
      float value;
      int lineNum;
   };
   std::array<UIControlModificationDisplay, 10> mUIControlModifications;
   
   class LineEventTracker
   {
   public:
      LineEventTracker()
      {
         for (size_t i=0; i<mTimes.size(); ++i)
            mTimes[i] = -999;
      }
      
      void AddEvent(int lineNum, string text = "")
      {
         if (lineNum >= 0 && lineNum < (int)mTimes.size())
         {
            mTimes[lineNum] = gTime;
            mText[lineNum] = text;
         }
      }
      
      void Draw(CodeEntry* codeEntry, int style, ofColor color);
   private:
      std::array<double, 256> mTimes;
      std::array<string, 256> mText;
   };
   
   LineEventTracker mLineExecuteTracker;
   LineEventTracker mMethodCallTracker;
   LineEventTracker mNotePlayTracker;
   LineEventTracker mUIControlTracker;

   struct BoundModuleConnection
   {
      int mLineIndex;
      string mLineText;
      IDrawableModule* mTarget;
   };
   std::vector<BoundModuleConnection> mBoundModuleConnections;
   
   std::vector<string> mScriptFilePaths;
   
   std::vector<PatchCableSource*> mExtraNoteOutputs;
   std::array<ModulationChain, 128> mPitchBends;
   std::array<ModulationChain, 128> mModWheels;
   std::array<ModulationChain, 128> mPressures;
   std::list<string> mMidiMessageQueue;
   ofMutex mMidiMessageQueueMutex;
   
   bool mShowJediWarning;
};
