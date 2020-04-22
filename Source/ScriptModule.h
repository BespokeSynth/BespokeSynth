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
#include "INoteSource.h"
#include "IPulseReceiver.h"

class ScriptModule : public IDrawableModule, public IButtonListener, public INoteSource, public IPulseReceiver
{
public:
   ScriptModule();
   virtual ~ScriptModule();
   static IDrawableModule* Create() { return new ScriptModule(); }
   
   string GetTitleLabel() override { return "script"; }
   void CreateUIControls() override;
   
   void Poll() override;
   
   void PlayNoteFromScript(int pitch, int velocity);
   
   void OnPulse(float amount, int samplesTo, int flags) override;
   void ButtonClicked(ClickButton* button) override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
   static ScriptModule* sCurrentScriptModule;
   
private:
   void RunScript(double time);
   
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
   double mScheduledRunTime;
   double mMostRecentRunTime;
};
