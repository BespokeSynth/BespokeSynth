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

namespace py = pybind11;

class ScriptModule : public IDrawableModule, public IButtonListener, public NoteEffectBase, public IPulseReceiver, public ICodeEntryListener
{
public:
   ScriptModule();
   virtual ~ScriptModule();
   static IDrawableModule* Create() { return new ScriptModule(); }
   
   string GetTitleLabel() override { return "script"; }
   void CreateUIControls() override;
   
   void Poll() override;
   
   void PlayNoteFromScript(int pitch, int velocity, float measureTime = -1);
   
   void OnPulse(float amount, int samplesTo, int flags) override;
   void ButtonClicked(ClickButton* button) override;
   
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
   
   float mWidth;
   float mHeight;
   double mScheduledPulseTime;
   double mMostRecentRunTime;
   
   py::object mPythonGlobals;
   
   struct ScheduledNoteOutput
   {
      double time;
      int pitch;
      int velocity;
   };
   static const int kScheduledNoteOutputBufferSize = 20;
   ScheduledNoteOutput mScheduledNoteOutput[kScheduledNoteOutputBufferSize];
   
   struct PendingNoteInput
   {
      double time;
      int pitch;
      int velocity;
   };
   static const int kPendingNoteInputBufferSize = 20;
   PendingNoteInput mPendingNoteInput[kPendingNoteInputBufferSize];
};
