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

    AbletonMoveControl.h
    Created: 22 Apr 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "MidiDevice.h"
#include "MidiController.h"
#include "TitleBar.h"
#include "DropdownList.h"
#include "AbletonMoveLCD.h"
#include "AbletonDeviceShared.h"

class IUIControl;
class Snapshots;
class ControlRecorder;
class TrackOrganizer;

class AbletonMoveControl : public IDrawableModule, public MidiDeviceListener, public IDropdownListener, public IAbletonGridDevice
{
public:
   AbletonMoveControl();
   virtual ~AbletonMoveControl();
   static IDrawableModule* Create() { return new AbletonMoveControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;
   void Exit() override;
   void KeyPressed(int key, bool isRepeat) override;

   void SetLed(MidiMessageType type, int index, int color, int flashColor = -1) override;
   bool GetButtonState(MidiMessageType type, int index) const override;
   void SetDisplayModule(IDrawableModule* module, bool addToHistory = true) override;
   void SetDisplayModuleWithContext(IDrawableModule* module, std::string context);
   void DisplayScreenMessage(std::string message, float durationMs = 500) override;
   IDrawableModule* GetDisplayModule() const override { return mDisplayModule; }
   AbletonDeviceType GetAbletonDeviceType() const override { return AbletonDeviceType::Move; }
   int GetGridStartIndex() const override { return AbletonDevice::kMovePadsSection; }
   int GetGridNumPads() const override { return AbletonDevice::kNumMovePads; }
   int GetGridNumCols() const override { return 8; }
   int GetGridNumRows() const override { return 4; }

   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;

   MidiDevice* GetDevice() override { return &mDevice; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   int GetModuleSaveStateRev() const override { return 2; }

   int GetGridControllerOption1Control() const override;
   int GetGridControllerOption2Control() const override;

   static IUIControl* sBindToUIControl;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void PostRender() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void OnClicked(float x, float y, bool right) override;

   bool Initialize();
   void DrawToFramebuffer();
   void RenderPush2Display();
   void UpdateLeds();
   void SendLeds();

   void UpdateControlList();
   int GetControlOffset() const;

   void DrawDisplayModuleRect(ofRectangle rect, float thickness);
   void SetGridControlInterface(IAbletonGridController* controller, IDrawableModule* module);
   void SetActiveTrackRow(int row);
   TrackOrganizer* GetActiveTrackRow() const;
   void AdjustGlobalModuleIndex(int amount);
   IDrawableModule* GetCurrentGlobalModule() const;
   void AdjustControlWithEncoder(IUIControl* control, float midiInputValue);
   int GetDisplayKnobIndex();
   bool ShouldDisplayMixer();
   bool ShouldDisplaySnapshotView();

   AbletonMoveLCD mLCD;
   double mScreenOverrideTimeout{ 0.0 };

   float mWidth{ 100 };
   float mHeight{ 100 };

   IDrawableModule* mDisplayModule{ nullptr };
   std::string mDisplayModuleContext{};
   std::vector<IUIControl*> mControls;
   std::vector<IUIControl*> mDisplayedControls;
   bool mDisplayModuleIsShowingOverrideControls{ false };
   float mModuleViewOffset{ 0 };

   std::array<PatchCableSource*, 8> mTrackCables;
   int mTrackRowOffset{ 0 };
   int mSelectedTrackRow{ -1 };
   int mPreviousSelectedTrackRow{ -1 };
   double mLastTrackSelectButtonPressTime{ 0 };

   std::array<PatchCableSource*, 5> mGlobalModuleCables{};
   int mGlobalModuleIndex{ 0 };

   PatchCableSource* mSongBuilderCable{ nullptr };

   bool mShiftHeld{ false };
   int mMostRecentlyTouchedKnobIndex{ -1 };
   int mLastAdjustedKnobIndex{ -1 };
   double mLastAdjustedKnobTime{ -1 };
   double mLastResetTime{ -1 };
   int mLastGainAdjustTrackIndex{ -1 };
   double mLastGainAdjustTrackTime{ -1 };
   float mTrackRowOffsetSmoothed{ 0 };

   IAbletonGridController* mGridControlInterface{ nullptr };

   struct LedState
   {
      LedState() {}
      LedState(int _color, int _flashColor)
      : color(_color)
      , flashColor(_flashColor)
      {}
      bool operator==(LedState& other) const
      {
         return color == other.color && flashColor == other.flashColor;
      }
      bool operator!=(LedState& other) const
      {
         return color != other.color || flashColor != other.flashColor;
      }
      int color{ -1 };
      int flashColor{ -1 };
   };

   std::array<LedState, 128 * 2> mQueuedLedState{}; //bottom 128 are notes, top 128 are CCs
   std::array<LedState, 128 * 2> mLedState{}; //bottom 128 are notes, top 128 are CCs
   std::array<int, 128 * 2> mButtonState{}; //bottom 128 are notes, top 128 are CCs

   MidiDevice mDevice;

   std::string mPushBridgeInitErrMsg;
};
