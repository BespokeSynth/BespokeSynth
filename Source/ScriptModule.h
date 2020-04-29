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
#include "pybind11/embed.h"
#include "Slider.h"

namespace py = pybind11;

class ScriptModule : public IDrawableModule, public IButtonListener, public NoteEffectBase, public IPulseReceiver, public ICodeEntryListener, public IFloatSliderListener
{
public:
   ScriptModule();
   virtual ~ScriptModule();
   static IDrawableModule* Create() { return new ScriptModule(); }
   
   static void ResetPython();
   
   string GetTitleLabel() override { return "script"; }
   void CreateUIControls() override;
   
   void Poll() override;
   
   void PlayNoteFromScript(int pitch, int velocity, float pan = 0);
   void PlayNoteFromScriptAfterDelay(int pitch, int velocity, float delayMeasureTime);
   void PlayNoteFromScriptAtMeasureTime(int pitch, int velocity, float measureTime);
   
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
   
   static ScriptModule* sCurrentScriptModule;
   
private:
   void RunScript(double time);
   void RunCode(double time, string code);
   void FixUpCode(string& code);
   void ScheduleNote(double time, int pitch, int velocity);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& width, int& height) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   CodeEntry* mCodeEntry;
   ClickButton* mRunButton;
   FloatSlider* mASlider;
   FloatSlider* mBSlider;
   FloatSlider* mCSlider;
   FloatSlider* mDSlider;
   float mA;
   float mB;
   float mC;
   float mD;
   
   float mWidth;
   float mHeight;
   double mScheduledPulseTime;
   double mMostRecentRunTime;
   string mLastError;
   
   py::object mPythonGlobals;
   
   struct ScheduledNoteOutput
   {
      double time;
      int pitch;
      int velocity;
   };
   static const int kScheduledNoteOutputBufferSize = 50;
   ScheduledNoteOutput mScheduledNoteOutput[kScheduledNoteOutputBufferSize];
   
   struct PendingNoteInput
   {
      double time;
      int pitch;
      int velocity;
   };
   static const int kPendingNoteInputBufferSize = 50;
   PendingNoteInput mPendingNoteInput[kPendingNoteInputBufferSize];
};
