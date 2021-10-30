//
//  M185Sequencer.h
//  modularSynth
//
//  Created by Lionel Landwerlin on 07/22/21.
//
//

#ifndef __modularSynth__M185Sequencer__
#define __modularSynth__M185Sequencer__

#include <iostream>
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

   
   void CreateUIControls() override;
   void Init() override;

   //IButtonListener
   void ButtonClicked(ClickButton* button) override;

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
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }

   void StepBy(double time, float velocity, int flags);
   void ResetStep();

   enum GateType
   {
      kGate_Repeat,
      kGate_Once,
      kGate_Hold,
      kGate_Rest,
   };

   struct Step
   {
      Step()
      : mPitch(0)
      , mPulseCount(1)
      , mGate(kGate_Repeat)
      , xPos(0)
      , yPos(0)
      , mPitchSlider(nullptr)
      , mPulseCountSlider(nullptr)
      , mGateSelector(nullptr)
      {
      }

      int mPitch;
      int mPulseCount;
      GateType mGate;

      float xPos, yPos;

      IntSlider* mPitchSlider;
      IntSlider* mPulseCountSlider;
      DropdownList* mGateSelector;
   };

   std::array<Step, NUM_M185SEQUENCER_STEPS> mSteps;
   float mWidth, mHeight;
   bool mHasExternalPulseSource;

   // Going through 0..(mSteps.size() - 1)
   int mStepIdx;
   int mLastPlayedStepIdx;

   // Going through 0..(mSteps[X].mPulseCount - 1)
   int mStepPulseIdx;

   int mLastPitch;

   NoteInterval mInterval;

   DropdownList* mIntervalSelector;

   ClickButton* mResetStepButton;
};

#endif /* defined(__modularSynth__M185Sequencer__) */
