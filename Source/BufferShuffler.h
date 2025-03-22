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
//
//  BufferShuffler.h
//
//  Created by Ryan Challinor on 6/23/23.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "INoteReceiver.h"
#include "SwitchAndRamp.h"
#include "Push2Control.h"
#include "GridController.h"

class BufferShuffler : public IAudioProcessor, public IDrawableModule, public INoteReceiver, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IPush2GridController, public IGridControllerListener
{
public:
   BufferShuffler();
   virtual ~BufferShuffler();
   static IDrawableModule* Create() { return new BufferShuffler(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void Poll() override;
   void CreateUIControls() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = MAX(w, 245);
      mHeight = MAX(h, 85);
   }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   void UpdateGridControllerLights(bool force);

   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void DropdownUpdated(DropdownList* dropdown, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }
   bool HasDebugDraw() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void OnClicked(float x, float y, bool right) override;
   bool DrawToPush2Screen() override;
   float GetFourTetPosition(double time);

   enum class PlaybackStyle
   {
      Normal,
      Double,
      Half,
      Reverse,
      DoubleReverse,
      HalfReverse,
      None
   };

   int GetWritePositionInSamples(double time);
   int GetLengthInSamples();
   void DrawBuffer(float x, float y, float w, float h);
   void PlayOneShot(int slice);
   int GetNumSlices();
   float GetSlicePlaybackRate() const;
   PlaybackStyle VelocityToPlaybackStyle(int velocity) const;

   ChannelBuffer mInputBuffer;

   float mWidth{ 245 };
   float mHeight{ 85 };
   int mNumBars{ 1 };
   IntSlider* mNumBarsSlider{ nullptr };
   NoteInterval mInterval{ kInterval_8n };
   DropdownList* mIntervalSelector{ nullptr };
   bool mFreezeInput{ false };
   Checkbox* mFreezeInputCheckbox{ nullptr };
   PlaybackStyle mPlaybackStyle{ PlaybackStyle::Normal };
   DropdownList* mPlaybackStyleDropdown{ nullptr };
   float mFourTet{ 0 };
   FloatSlider* mFourTetSlider{ nullptr };
   int mFourTetSlices{ 4 };
   DropdownList* mFourTetSlicesDropdown{ nullptr };
   int mQueuedSlice{ -1 };
   PlaybackStyle mQueuedPlaybackStyle{ PlaybackStyle::None };
   float mPlaybackSampleIndex{ -1 };
   double mPlaybackSampleStartTime{ -1 };
   double mPlaybackSampleStopTime{ -1 };
   GridControlTarget* mGridControlTarget{ nullptr };
   bool mUseVelocitySpeedControl{ false };
   bool mOnlyPlayWhenTriggered{ false };
   float mFourTetSampleIndex{ 0 };

   SwitchAndRamp mSwitchAndRamp;
};
