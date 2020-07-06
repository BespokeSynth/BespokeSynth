//
//  NoteLooper.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/31/13.
//
//

#ifndef __modularSynth__NoteLooper__
#define __modularSynth__NoteLooper__

#include <iostream>
#include "NoteEffectBase.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "MidiDevice.h"
#include "Scale.h"

#define NOTELOOPER_MAX_NOTES 500
#define NOTELOOPER_MAX_CHORD 10
#define NOTELOOPER_NOTE_RANGE 127

class NoteLooper : public IDrawableModule, public NoteEffectBase, public IAudioPoller, public IFloatSliderListener, public IButtonListener, public IIntSliderListener, public IDropdownListener
{
public:
   NoteLooper();
   ~NoteLooper();
   static IDrawableModule* Create() { return new NoteLooper(); }
   
   string GetTitleLabel() override { return "note looper"; }
   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void Clear();
   void Retrigger();
   void TriggerRecordedNote(int pitch, int velocity);
   void RecordNote(double time, int pitch, int velocity);
   void StopRecording();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=305; height=140; }
   bool Enabled() const override { return true; }
   
   struct NoteEvent
   {
      bool mValid;
      float mPos;
      int mPitch;
      int mVelocity;
      int mJustPlaced;
      int mAssociatedEvent;   //associated note on/off
   };
   
   struct CurrentNote
   {
      int mPitch;
      int mVelocity;
   };

   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mRecord;
   Checkbox* mRecordCheckbox;
   IntSlider* mNumBarsSlider;
   int mOctave;
   int mNumBars;
   ClickButton* mClearButton;
   IntSlider* mOctaveSlider;
   NoteEvent mNoteroll[NOTELOOPER_MAX_NOTES];
   CurrentNote mCurrentNotes[NOTELOOPER_MAX_CHORD];
   bool mOverdub;
   Checkbox* mOverdubCheckbox;
   bool mHeldNotes[127];
   int mNumHeldNotes;
   ClickButton* mNumBarsDecrement;
   ClickButton* mNumBarsIncrement;
};

#endif /* defined(__modularSynth__NoteLooper__) */

