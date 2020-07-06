//
//  LaunchpadKeyboard.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#ifndef __modularSynth__LaunchpadKeyboard__
#define __modularSynth__LaunchpadKeyboard__

#include <iostream>
#include "Scale.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "GridController.h"
#include "Push2Control.h"

class LaunchpadNoteDisplayer;

class Chorder;

class LaunchpadKeyboard : public IDrawableModule, public INoteSource, public IScaleListener, public IIntSliderListener, public ITimeListener, public IDropdownListener, public IFloatSliderListener, public IGridControllerListener, public IPush2GridController
{
public:
   LaunchpadKeyboard();
   ~LaunchpadKeyboard();
   static IDrawableModule* Create() { return new LaunchpadKeyboard(); }
   
   string GetTitleLabel() override { return "gridkeyboard"; }
   void CreateUIControls() override;

   void SetDisplayer(LaunchpadNoteDisplayer* displayer) { mDisplayer = displayer; }
   void DisplayNote(int pitch, int velocity);
   void SetChorder(Chorder* chorder) { mChorder = chorder; }
   
   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   //IScaleListener
   void OnScaleChanged() override;
   
   //IDrawableModule
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   void Exit() override;
   void Poll() override;

   //ITimeListener
   void OnTimeEvent(double time) override;
   
   //IPush2GridController
   bool OnPush2Control(MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   enum LaunchpadLayout
   {
      kChromatic,
      kDiatonic,
      kChordIndividual,
      kChord,
      kGuitar,
      kSeptatonic
   };
   
   enum ArrangementMode
   {
      kFull,
      kFive,
      kSix
   };

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=74; }
   
   void UpdateLights(bool force = false);
   GridColor GetGridSquareColor(int x, int y);
   int GridToPitch(int x, int y);
   void HandleChordButton(int pitch, bool bOn);
   bool IsChordButtonPressed(int pitch);
   void PressedNoteFor(int x, int y, int velocity);
   void ReleaseNoteFor(int x, int y);
   int GridToPitchChordSection(int x, int y);
   int GetHeldVelocity(int pitch) { if (pitch >= 0 && pitch < 128) return mCurrentNotes[pitch]; else return 0; }
   
   int mRootNote;
   
   int mCurrentNotes[128];
   bool mTestKeyHeld;
   int mOctave;
   IntSlider* mOctaveSlider;
   bool mLatch;
   Checkbox* mLatchCheckbox;
   LaunchpadLayout mLayout;
   DropdownList* mLayoutDropdown;
   int mCurrentChord;
   vector< vector<int> > mChords;
   LaunchpadNoteDisplayer* mDisplayer;
   ArrangementMode mArrangementMode;
   DropdownList* mArrangementModeDropdown;
   list<int> mHeldChordTones;
   Chorder* mChorder;
   bool mLatchChords;
   Checkbox* mLatchChordsCheckbox;
   bool mWasChorderEnabled;
   bool mPreserveChordRoot;
   Checkbox* mPreserveChordRootCheckbox;
   GridController* mGridController;
};

#endif /* defined(__modularSynth__LaunchpadKeyboard__) */

