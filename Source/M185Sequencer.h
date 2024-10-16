//
//  M185Sequencer.h
//  modularSynth
//
//  Created by Lionel Landwerlin on 07/22/21.
//
//

#pragma once

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "INoteSource.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "IDrivableSequencer.h"

#define NUM_M185SEQUENCER_STEPS 8

class M185Sequencer : public IDrawableModule, public IButtonListener, public IDropdownListener, public IIntSliderListener, public ITimeListener, public IPulseReceiver, public INoteSource, public IDrivableSequencer
{
public:
   M185Sequencer();
   virtual ~M185Sequencer();
   static IDrawableModule* Create() { return new M185Sequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   //IDrawableModule
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void StepBy(double time, float velocity, int flags);
   void ResetStep();
   void FindNextStep();

   enum GateType
   {
      kGate_Repeat,
      kGate_Once,
      kGate_Hold,
      kGate_Rest,
   };

   struct Step
   {
      Step() {}

      int mPitch{ 0 };
      int mPulseCount{ 0 };
      GateType mGate{ GateType::kGate_Repeat };

      float xPos{ 0 }, yPos{ 0 };

      IntSlider* mPitchSlider{ nullptr };
      IntSlider* mPulseCountSlider{ nullptr };
      DropdownList* mGateSelector{ nullptr };
   };

   std::array<Step, NUM_M185SEQUENCER_STEPS> mSteps;
   float mWidth{ 0 }, mHeight{ 0 };
   bool mHasExternalPulseSource{ false };
   TransportListenerInfo* mTransportListenerInfo{ nullptr };

   // Going through 0..(mSteps.size() - 1)
   int mStepIdx{ 0 };
   int mLastPlayedStepIdx{ 0 };

   // Going through 0..(mSteps[X].mPulseCount - 1)
   int mStepPulseIdx{ 0 };

   int mLastPitch{ 0 };

   NoteInterval mInterval{ NoteInterval::kInterval_8n };

   DropdownList* mIntervalSelector{ nullptr };

   ClickButton* mResetStepButton{ nullptr };
};
