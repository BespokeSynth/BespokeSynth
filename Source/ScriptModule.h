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

class ScriptModule : public IDrawableModule, public IButtonListener, public NoteEffectBase, public IPulseReceiver, public ICodeEntryListener, public IFloatSliderListener, public IDropdownListener
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
   
   void PlayNoteFromScript(int pitch, int velocity, float pan);
   void PlayNoteFromScriptAfterDelay(int pitch, int velocity, float delayMeasureTime, float pan);
   void ScheduleMethod(string method, float delayMeasureTime);
   void ScheduleUIControlValue(IUIControl* control, float value, float delayMeasureTime);
   void HighlightLine(int lineNum, int scriptModuleIndex);
   void PrintText(string text);
   IUIControl* GetUIControl(string path);
   
   void OnPulse(double time, float velocity, int flags) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldValue) override {}
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldValue) override {}
 
   //ICodeEntryListener
   void ExecuteCode(string code) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   bool HasDebugDraw() const override { return true; }
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;

   static std::vector<ScriptModule*> sScriptModules;
   static ScriptModule* sLastLineExecutedModule;
   static float GetScriptMeasureTime();
   static float GetTimeSigRatio();
   
private:
   void PlayNote(double time, int pitch, int velocity, float pan, int lineNum);
   void AdjustUIControl(IUIControl* control, float value, int lineNum);
   void RunScript(double time);
   void RunCode(double time, string code);
   void FixUpCode(string& code);
   void ScheduleNote(double time, int pitch, int velocity, float pan);
   string GetThisName();
   string GetIndentation(string line);
   bool ShouldDisplayLineExecutionPre(string priorLine, string line);
   bool ShouldDisplayLineExecutionPost(string line);
   void GetFirstAndLastCharacter(string line, char& first, char& last);
   bool IsNonWhitespace(string line);
   void DrawTimer(int lineNum, double startTime, double endTime, ofColor color, bool filled);
   void RefreshScriptFiles();
   void Stop();
   
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
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
   
   struct ScheduledNoteOutput
   {
      double startTime;
      double time;
      int pitch;
      int velocity;
      float pan;
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
   
   std::vector<string> mScriptFilePaths;
};
