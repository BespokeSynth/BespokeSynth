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

class ScriptModule : public IDrawableModule, public IButtonListener, public NoteEffectBase, public IPulseReceiver, public ICodeEntryListener, public IFloatSliderListener
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
   void PlayNoteFromScriptAfterDelay(int pitch, int velocity, float delayMeasureTime);
   void ScheduleMethod(string method, float delayMeasureTime);
   void HighlightLine(int lineNum);
   void PrintText(string text);
   IUIControl* GetUIControl(string path);
   void OnAdjustUIControl(IUIControl* control, float value);
   
   void OnPulse(float amount, int samplesTo, int flags) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldValue) override {}
   
   //ICodeEntryListener
   void ExecuteCode(string code) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
   static std::vector<ScriptModule*> sScriptModules;
   static float GetScriptMeasureTime();
   
private:
   void PlayNote(double time, int pitch, int velocity, float pan, int lineNum);
   void RunScript(double time);
   void RunCode(double time, string code);
   void FixUpCode(string& code);
   void ScheduleNote(double time, int pitch, int velocity);
   string GetThisName();
   string GetIndentation(string line);
   bool ShouldDisplayLineExecutionPre(string priorLine, string line);
   bool ShouldDisplayLineExecutionPost(string line);
   void GetFirstAndLastCharacter(string line, char& first, char& last);
   bool IsNonWhitespace(string line);
   void DrawTimer(int lineNum, double startTime, double endTime, ofColor color);
   
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& width, int& height) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   CodeEntry* mCodeEntry;
   ClickButton* mRunButton;
   FloatSlider* mASlider;
   FloatSlider* mBSlider;
   FloatSlider* mCSlider;
   FloatSlider* mDSlider;
   Checkbox* mDebugCheckbox;
   float mA;
   float mB;
   float mC;
   float mD;
   
   float mWidth;
   float mHeight;
   double mScheduledPulseTime;
   static double sMostRecentRunTime;
   string mLastError;
   size_t mScriptModuleIndex;
   bool mDrawDebug;
   string mLastRunLiteralCode;
   int mNextLineToExecute;
   
   struct ScheduledNoteOutput
   {
      double startTime;
      double time;
      int pitch;
      int velocity;
      int lineNum;
   };
   static const int kScheduledNoteOutputBufferSize = 50;
   ScheduledNoteOutput mScheduledNoteOutput[kScheduledNoteOutputBufferSize];
   
   struct ScheduledMethodCall
   {
      double startTime;
      double time;
      string method;
      int lineNum;
   };
   static const int kScheduledMethodCallBufferSize = 50;
   ScheduledMethodCall mScheduledMethodCall[kScheduledMethodCallBufferSize];
   
   struct PendingNoteInput
   {
      double time;
      int pitch;
      int velocity;
   };
   static const int kPendingNoteInputBufferSize = 50;
   PendingNoteInput mPendingNoteInput[kPendingNoteInputBufferSize];
   
   struct PrintDisplay
   {
      double time;
      string text;
      int lineNum;
   };
   static const int kPrintDisplayBufferSize = 10;
   PrintDisplay mPrintDisplay[kPrintDisplayBufferSize];
   
   struct UIControlModificationDisplay
   {
      double time;
      ofVec2f position;
      float value;
      int lineNum;
   };
   static const int kUIControlModificationBufferSize = 10;
   UIControlModificationDisplay mUIControlModifications[kUIControlModificationBufferSize];
   
   class LineEventTracker
   {
   public:
      LineEventTracker()
      {
         for (int i=0; i<kNumLineTrackers; ++i)
            mTimes[i] = -999;
      }
      
      void AddEvent(int lineNum, string text = "")
      {
         if (lineNum >= 0 && lineNum < kNumLineTrackers)
         {
            mTimes[lineNum] = gTime;
            mText[lineNum] = text;
         }
      }
      
      void Draw(CodeEntry* codeEntry, int style, ofColor color);
   private:
      static const int kNumLineTrackers = 256;
      double mTimes[kNumLineTrackers];
      string mText[kNumLineTrackers];
   };
   
   LineEventTracker mLineExecuteTracker;
   LineEventTracker mMethodCallTracker;
   LineEventTracker mNotePlayTracker;
   LineEventTracker mUIControlTracker;
};
