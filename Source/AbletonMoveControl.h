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
#include "pybind11/attr.h"

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

   void SetLed(int index, int color, int flashColor = -1) override;
   void SetLedFlashColor(int index, int flashColor = -1);
   bool GetButtonState(int index) const override;
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
   void OnMidiPressure(MidiPressure& pressure) override;

   MidiDevice* GetDevice() override { return &mDevice; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   int GetModuleSaveStateRev() const override { return 3; }

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
   void SendLeds(bool force);

   void UpdateControlList();
   static bool IsDisplayableControl(IUIControl* control);
   int GetControlOffset() const;

   void DrawDisplayModuleRect(ofRectangle rect, float thickness);
   void SetGridControlInterface(IAbletonGridController* controller);
   void SetActiveTrackRow(int row, bool resetModuleIndex);
   TrackOrganizer* GetActiveTrackRow() const;
   bool AdjustGlobalModuleIndex(int amount);
   IDrawableModule* GetCurrentGlobalModule() const;
   IAbletonGridController* GetCurrentGlobalGridInterface() const;
   void AdjustControlWithEncoder(IUIControl* control, float midiInputValue, bool ignoreShift = false);
   int GetDisplayKnobIndex();
   bool ShouldDisplayMixer();
   bool ShouldDisplaySnapshotView();
   float GetModuleViewOffset() const;
   void SetModuleViewOffset(float offset);
   void DetermineTrackControlLayout();
   bool WasPeekHold(int controlIndex) const { return gTime - mControlState[controlIndex].mLastChangeTime > 300; }
   void ZoomToTrack(TrackOrganizer* track);

   const int kTrackRowGlobal = -1;
   const int kTrackRowMixer = -2;
   const int kTrackRowTransport = -3;
   const int kTrackRowScale = -4;

   AbletonMoveLCD mLCD;
   double mScreenOverrideTimeout{ 0.0 };
   std::string mTemporaryScreenMessage{};
   double mTemporaryScreenMessageTimeout{ 0.0 };
   bool mShowSoundSelector{ false };
   int mSoundSelectorIndex{ 0 };

   IDrawableModule* mDisplayModule{ nullptr };
   std::string mDisplayModuleContext{};
   std::vector<IUIControl*> mControls;
   bool mDisplayModuleIsShowingOverrideControls{ false };

   std::array<PatchCableSource*, 8> mTrackCables{ nullptr };
   int mTrackRowOffset{ 0 };
   int mSelectedTrackRow{ kTrackRowMixer };
   int mPreviousSelectedTrackRow{ -1 };
   double mDisplayModuleSelectTimeout{ 0.0 };
   int mSnapshotOffset{ 0 };

   static constexpr int kNumPages{ 5 };
   std::array<PatchCableSource*, kNumPages> mGlobalControlModuleCables{};
   std::array<PatchCableSource*, kNumPages> mGlobalGridInterfaceCables{};
   int mGlobalModuleIndex{ 0 };
   float mGlobalModuleViewOffset{ 0.0f };

   struct TrackLayoutEntry
   {
      TrackLayoutEntry(std::string name, int pageCount)
      : mName(name)
      , mPageCount(pageCount)
      {}

      std::string mName{};
      int mPageCount{ 0 };
   };

   std::vector<TrackLayoutEntry> mTrackControlLayout{};

   enum class LissajousDisplayMode
   {
      Always,
      MixerTrack,
      Never
   };
   LissajousDisplayMode mLissajousDisplayMode{ LissajousDisplayMode::MixerTrack };

   PatchCableSource* mSongBuilderCable{ nullptr };
   PatchCableSource* mOutputGainCable{ nullptr };

   bool mShiftHeld{ false };
   int mMostRecentlyTouchedKnobIndex{ -1 };
   int mLastAdjustedKnobIndex{ -1 };
   double mLastAdjustedKnobTime{ -1 };
   double mLastResetTime{ -1 };
   int mLastGainAdjustTrackIndex{ -1 };
   double mLastGainAdjustTrackTime{ -1 };
   float mTrackRowOffsetSmoothed{ 0 };
   bool mShowLCDOnScreen{ false };
   bool mAutoZoomToTrack{ false };
   bool mPageWithinModules{ false };
   bool mBottomRowMode{ false };

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

   struct ControlState
   {
      LedState mQueuedLedState{};
      LedState mLedState{};
      int mButtonState{ 0 };
      double mLastChangeTime{ 0.0 };
   };

   std::array<ControlState, 128 * 2> mControlState{}; //bottom 128 are notes, top 128 are CCs

   MidiDevice mDevice;

   std::string mPushBridgeInitErrMsg;
};
