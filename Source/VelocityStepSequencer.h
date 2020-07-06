//
//  VelocityStepSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/14/14.
//
//

#ifndef __Bespoke__VelocityStepSequencer__
#define __Bespoke__VelocityStepSequencer__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"
#include "MidiController.h"
#include "INoteReceiver.h"

#define VSS_MAX_STEPS 8
#define VSS_RANGE 127

class VelocityStepSequencer : public IDrawableModule, public ITimeListener, public NoteEffectBase, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public MidiDeviceListener
{
public:
   VelocityStepSequencer();
   ~VelocityStepSequencer();
   static IDrawableModule* Create() { return new VelocityStepSequencer(); }
   
   string GetTitleLabel() override { return "velocity seq"; }
   void CreateUIControls() override;
   
   void SetMidiController(string name);
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 160; height = 160; }
   bool Enabled() const override { return mEnabled; }
   
   int mVels[VSS_MAX_STEPS];
   IntSlider* mVelSliders[VSS_MAX_STEPS];
   
   NoteInterval mInterval;
   int mArpIndex;
   
   DropdownList* mIntervalSelector;
   bool mResetOnDownbeat;
   Checkbox* mResetOnDownbeatCheckbox;
   int mCurrentVelocity;
   
   int mLength;
   IntSlider* mLengthSlider;
   bool mSetLength;
   
   MidiController* mController;
};

#endif /* defined(__Bespoke__VelocityStepSequencer__) */

