/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    PlaySequencer.h
    Created: 12 Dec 2020 11:00:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IDrawableModule
   void Init() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;

   void SetGridSize(float w, float h);
   int GetStep(double time);
   void UpdateInterval();
   void UpdateNumMeasures(int oldNumMeasures);
   void UpdateLights(bool betweener = false);
   int GetVelocityLevel();

   class NoteOffScheduler : public ITimeListener
   {
   public:
      //ITimeListener
      void OnTimeEvent(double time) override;
      PlaySequencer* mOwner{ nullptr };
   };

   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   int mNumMeasures{ 1 };
   bool mWrite{ false };
   bool mNoteRepeat{ false };
   bool mLinkColumns{ false };
   float mWidth{ 240 };
   float mHeight{ 20 };
   bool mUseLightVelocity{ false };
   bool mUseMedVelocity{ false };
   bool mClearLane{ false };
   bool mSustain{ false };
   float mVelocityFull{ 1 };
   float mVelocityMed{ .5 };
   float mVelocityLight{ .25 };

   DropdownList* mIntervalSelector{ nullptr };
   Checkbox* mWriteCheckbox{ nullptr };
   Checkbox* mNoteRepeatCheckbox{ nullptr };
   Checkbox* mLinkColumnsCheckbox{ nullptr };
   DropdownList* mNumMeasuresSelector{ nullptr };
   UIGrid* mGrid{ nullptr };
   GridControlTarget* mGridControlTarget{ nullptr };
   NoteOffScheduler mNoteOffScheduler;

   struct PlayLane
   {
      int mInputVelocity{ 0 };
      bool mIsPlaying{ false };
      Checkbox* mMuteOrEraseCheckbox{ nullptr };
      bool mMuteOrErase{ false };
   };

   std::array<PlayLane, 16> mLanes;

   struct SavedPattern
   {
      ClickButton* mStoreButton{ nullptr };
      ClickButton* mLoadButton{ nullptr };
      float mNumMeasures{ 1 };
      std::array<float, MAX_GRID_COLS * MAX_GRID_ROWS> mData{};
      bool mHasSequence{ false };
   };

   std::array<SavedPattern, 5> mSavedPatterns;

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
