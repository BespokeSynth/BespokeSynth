//
//  Arpeggiator.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef __modularSynth__Arpeggiator__
#define __modularSynth__Arpeggiator__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"
#include "Grid.h"
#include "Scale.h"
#include "ModulationChain.h"

class Arpeggiator : public NoteEffectBase, public IDrawableModule, public ITimeListener, public ITextEntryListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IScaleListener
{
public:
   Arpeggiator();
   ~Arpeggiator();
   static IDrawableModule* Create() { return new Arpeggiator(); }
   
   string GetTitleLabel() override { return "arpeggiator"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   //ITimeListener
   void OnTimeEvent(int samplesTo) override;
   
   //IScaleListener
   void OnScaleChanged() override;
   
   //ITextEntryListener
   void TextEntryComplete(TextEntry* entry) override;

   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 240; height = mViewGrid ? 225 : 120; }
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   void GenerateRandomArpeggio();
   void SyncGridToArp();
   string GetArpNoteDisplay(int note);
   
   void UpdateInterval();
   
   struct ArpNote
   {
      ArpNote(int _pitch, int _vel, int _voiceIdx, ModulationChain* _pitchBend, ModulationChain* _modWheel, ModulationChain* _pressure) : pitch(_pitch), vel(_vel), voiceIdx(_voiceIdx), pitchBend(_pitchBend), modWheel(_modWheel), pressure(_pressure) {}
      int pitch;
      int vel;
      int voiceIdx;
      ModulationChain* pitchBend;
      ModulationChain* modWheel;
      ModulationChain* pressure;
   };
   vector<ArpNote> mChord;
   
   bool mUseHeldNotes;
   NoteInterval mInterval;
   int mUserPitch;
   int mLastPitch;
   int mArpIndex;
   bool mRestartOnPress;
   char mArpString[MAX_TEXTENTRY_LENGTH];
   bool mPlayOnlyScaleNotes;
   Checkbox* mPlayOnlyScaleNotesCheckbox;
   
   DropdownList* mIntervalSelector;
   Checkbox* mUseHeldNotesCheckbox;
   TextEntry* mArpEntry;
   ClickButton* mEasyButton;
   bool mRepeatIsHold;
   Checkbox* mRepeatIsHoldCheckbox;
   Checkbox* mRestartOnPressCheckbox;
   int mArpStep;
   int mArpPingPongDirection;
   IntSlider* mArpStepSlider;
   bool mResetOnDownbeat;
   Checkbox* mResetOnDownbeatCheckbox;
   bool mViewGrid;
   Grid* mGrid;
   Checkbox* mViewGridCheckbox;
   
   int mRandomLength;
   IntSlider* mRandomLengthSlider;
   int mRandomRange;
   IntSlider* mRandomRangeSlider;
   bool mRandomRest;
   Checkbox* mRandomRestCheckbox;
   bool mRandomHold;
   Checkbox* mRandomHoldCheckbox;
   
   bool mUpbeats;
   Checkbox* mUpbeatsCheckbox;

   ofMutex mChordMutex;
};

#endif /* defined(__modularSynth__Arpeggiator__) */

