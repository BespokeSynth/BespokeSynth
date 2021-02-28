/*
  ==============================================================================

    PlaySequencer.h
    Created: 12 Dec 2020 11:00:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"
#include "UIGrid.h"
#include "Scale.h"
#include "ModulationChain.h"
#include "GridController.h"

class PlaySequencer : public NoteEffectBase, public IDrawableModule, public ITimeListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IGridControllerListener
{
public:
   PlaySequencer();
   ~PlaySequencer();
   static IDrawableModule* Create() { return new PlaySequencer(); }

   string GetTitleLabel() override { return "play sequencer"; }
   void CreateUIControls() override;

   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

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
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;

   int GetStep(double time);
   void UpdateInterval();
   void UpdateNumMeasures(int oldNumMeasures);
   void UpdateLights(bool betweener=false);
   int GetVelocityLevel();

   class NoteOffScheduler : public ITimeListener
   {
   public:
      //ITimeListener
      void OnTimeEvent(double time) override;
      PlaySequencer* mOwner;
   };

   NoteInterval mInterval;
   int mNumMeasures;
   bool mWrite;
   bool mNoteRepeat;
   bool mLinkColumns;
   float mWidth;
   float mHeight;
   bool mUseLightVelocity;
   bool mUseMedVelocity;
   bool mClearLane;
   bool mSustain;
   float mVelocityFull;
   float mVelocityMed;
   float mVelocityLight;
   
   DropdownList* mIntervalSelector;
   Checkbox* mWriteCheckbox;
   Checkbox* mNoteRepeatCheckbox;
   Checkbox* mLinkColumnsCheckbox;
   DropdownList* mNumMeasuresSelector;
   UIGrid* mGrid;
   GridController* mGridController;
   NoteOffScheduler mNoteOffScheduler;

   struct PlayLane
   {
      PlayLane() : mInputVelocity(0), mIsPlaying(false), mMuteOrErase(false) {}
      int mInputVelocity;
      bool mIsPlaying;
      Checkbox* mMuteOrEraseCheckbox;
      bool mMuteOrErase;
   };

   std::array<PlayLane, 16> mLanes;

   struct SavedPattern
   {
      SavedPattern() : mNumMeasures(1), mHasSequence(false) {}
      ClickButton* mStoreButton;
      ClickButton* mLoadButton;
      float mNumMeasures;
      std::array<float, MAX_GRID_SIZE*MAX_GRID_SIZE> mData;
      bool mHasSequence;
   };

   std::array<SavedPattern, 5> mSavedPatterns;
};