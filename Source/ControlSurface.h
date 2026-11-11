/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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

    ControlSurface.h
    Created: 24 Oct 2026
    Author:  Ryan Challinor

    Uses a hardware control surface (ableton move, ableton push 2, novation
    launch control 3, ...) to drive a sessionorganizer. The hardware specifics
    live in ControlSurfaceDevice.

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "MidiDevice.h"
#include "MidiController.h"
#include "DropdownList.h"
#include "AbletonMoveLCD.h"
#include "AbletonDeviceShared.h"
#include "ControlSurfaceDevice.h"
#include "LockFreeQueue.h"
#include "SessionOrganizer.h"

class IUIControl;
class ControlRecorder;
class Checkbox;
class TrackOrganizer;

class ControlSurface : public IDrawableModule, public MidiDeviceListener, public IDropdownListener, public IAbletonGridDevice
{
public:
   ControlSurface();
   virtual ~ControlSurface();
   static IDrawableModule* Create() { return new ControlSurface(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;
   void Exit() override;

   //IAbletonGridDevice
   void SetLed(int index, int color, int flashColor = -1, LedPriority priority = LedPriority::Normal) override;
   void SetLedFlashColor(int index, int flashColor) override;
   bool GetButtonState(int index) const override;
   int GetGridControllerOption1Control() const override;
   int GetGridControllerOption2Control() const override;
   MidiDevice* GetDevice() override { return &mDevice; }
   void SetDisplayModule(IDrawableModule* module, bool addToHistory = true) override;
   void SetDisplayModuleWithContext(IDrawableModule* module, std::string context);
   IDrawableModule* GetDisplayModule() const override { return mDisplayModule; }
   AbletonDeviceType GetAbletonDeviceType() const override { return AbletonDeviceType::Move; } //every device is presented to grid controllers with the move's layout
   int GetGridStartIndex() const override { return Surface()->GetGridStartIndex(); }
   int GetGridNumPads() const override { return Surface()->GetGridNumPads(); }
   int GetGridNumCols() const override { return Surface()->GetGridNumCols(); }
   int GetGridNumRows() const override { return Surface()->GetGridNumRows(); }
   void DisplayScreenMessage(std::string message, float durationMs = 500) override;

   //MidiDeviceListener
   void OnMidi(const juce::MidiMessage& message) override;
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;
   void OnMidiPressure(MidiPressure& pressure) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   void PostLoadState() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   int GetModuleSaveStateRev() const override { return 3; }

   static IUIControl* sBindToUIControl;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void PostRender() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool ShowsLCDOnModule() const { return !Surface()->HasDisplay(); } //devices without a screen get one on the module
   void OnClicked(float x, float y, bool right) override;

   ControlSurfaceDevice* Surface() const;
   int NumMainEncoders() const { return Surface()->GetNumMainEncoders(); }
   int EncoderPageSize() const { return Surface()->GetEncoderPageSize(); }
   int NumTrackButtons() const { return Surface()->GetNumTrackButtons(); }
   int NumStepButtons() const { return Surface()->GetNumStepButtons(); }
   bool HasGrid() const { return Surface()->HasGrid(); }
   int GetMaxTrackRowOffset() const;
   int GetNumSnapshotColumns() const;
   bool UseTrackButtonsForSnapshots() const;

   void AutoDetectDevice();
   void ConnectDevice();
   void DisconnectDevice();
   void DrawToFramebuffer(bool separatePanels = false);
   void DrawDisplayPanels();
   void DrawModuleList(AbletonMoveLCD& lcd);
   void DrawMixer(AbletonMoveLCD& lcd);
   bool CanSelectModules() const;
   FloatSlider* GetMixerSlider(int trackIndex) const;
   int GetMixerAdjustTrackIndex();
   void GetKnobColors(int controlIndex, int& color, int& flashColor) const;
   IUIControl* GetControlAt(int controlIndex) const;
   void UpdateLeds();
   void SetLedWithBrightness(int index, int color, int flashColor, float brightness, LedPriority priority = LedPriority::Normal);
   void SendLeds(bool force);

   void UpdateControlList();
   static bool IsDisplayableControl(IUIControl* control);
   int GetControlOffset() const;
   int GetNumControlPages() const;
   void StepControlPage(int direction);

   void DrawDisplayModuleRect(ofRectangle rect, float thickness);
   void DrawLCDImage(float x, float y, float width, float height, float alpha);
   void SetGridControlInterface(IAbletonGridController* controller);
   void SetActiveTrackRow(int row, bool resetModuleIndex);
   TrackOrganizer* GetActiveTrackRow() const;
   bool AdjustGlobalModuleIndex(int amount);
   IDrawableModule* GetCurrentGlobalModule() const;
   IAbletonGridController* GetCurrentGlobalGridInterface() const;
   void AdjustControlWithEncoder(IUIControl* control, float midiInputValue, bool ignoreShift = false);
   int GetDisplayKnobIndex();
   bool ShouldDisplayMixer();
   bool ShouldDisplaySnapshotView() const;
   float GetModuleViewOffset() const;
   void SetModuleViewOffset(float offset);
   void DetermineTrackControlLayout();
   bool WasPeekHold(int controlIndex) const { return gTime - mControlState[controlIndex].mLastChangeTime > 300; }
   void ZoomToTrack(TrackOrganizer* track);
   void ZoomToModule(IDrawableModule* module);
   void OnTrackRowExited(int newRow, int oldRow);
   void ShowSoundSelector();
   void ShowAddTrackSelector();
   void OnScalePageButton(int controlIndex, int velocity);
   void ToggleAutoZoomToTrack();
   void ToggleTrackEnabled(int trackIndex);
   void AddLFOToControl(int controlIndex);
   bool HandleDedicatedButton(int controlIndex, int velocity);
   bool HasDedicatedButton(int layoutIndex) const { return Surface()->HasInput(layoutIndex); }
   bool MuteButtonMutesTracks() const { return HasDedicatedButton(AbletonDevice::kLFOButton); } //when lfos have their own button, mute is free to mute tracks
   bool IsStoreSnapshotHeld() const;
   void TrackModifierUse(int index, int value);
   void MarkUsedSincePress(int index) { mControlState[index].mUsedSincePress = true; }
   bool IsHeldOnItsOwn(int index) const;
   void UpdateHoldHints();
   void DisplayHint(const std::string& hint);
   int GetBindButton() const { return HasDedicatedButton(AbletonDevice::kBindButton) ? AbletonDevice::kBindButton : AbletonDevice::kCaptureButton; }
   bool BindTouchedKnob();
   void StartControlRecorder(int controlIndex);
   void DrawKnobDisplay(AbletonMoveLCD& lcd, IUIControl* control);
   void DrawLissajousDisplay(AbletonMoveLCD& lcd, const TrackOrganizer* track);
   bool IsRandomizeChordHeld() const;
   IDrawableModule* GetCurrentControlModule() const;

   void HandleInput_InputThread(int rawIndex, int value);
   void OnNoteInput(int pitch, int velocity);
   void OnControlInput(int controlId, int value);
   void OnPitchBendInput(MidiPitchBend& pitchBend);
   void OnPressureInput(MidiPressure& pressure);

   const int kTrackRowGlobal = -1;
   const int kTrackRowMixer = -2;
   const int kTrackRowTransport = -3;
   const int kTrackRowScale = -4;

   ControlSurfaceDeviceType mDeviceType{ ControlSurfaceDeviceType::AbletonMove };
   DropdownList* mDeviceTypeDropdown{ nullptr };
   std::array<std::unique_ptr<ControlSurfaceDevice>, (int)ControlSurfaceDeviceType::Count> mSurfaces{};
   std::atomic<ControlSurfaceDevice*> mConnectedSurface{ nullptr };
   bool mNeedsConnect{ true };
   double mLastLedKeepAliveTime{ 0.0 };
   bool mLoadedFromState{ false };
   std::string mStatusMessage{};

   AbletonMoveLCD mLCD;
   //extra screens for devices that show several at once (push 2)
   std::array<AbletonMoveLCD, AbletonDevice::kNumMainEncoders> mKnobLCDs;
   AbletonMoveLCD mModuleListLCD;
   AbletonMoveLCD mMixerLCD;
   AbletonMoveLCD mLissajousLCD;
   std::vector<ControlSurfaceDisplayPanel> mDisplayPanels;
   std::array<int, (int)NanoVGRenderContext::Num> mLCDImages{}; //nanovg image of the lcd for each render context
   std::vector<unsigned char> mLCDImagePixels;
   double mScreenOverrideTimeout{ 0.0 };
   std::string mTemporaryScreenMessage{};
   double mTemporaryScreenMessageTimeout{ 0.0 };
   bool mTemporaryScreenMessageIsHint{ false };
   bool mShowSoundSelector{ false };
   int mSoundSelectorIndex{ 0 };

   bool mShowAddTrackSelector{ false };
   int mAddTrackSelectorIndex{ 0 };
   std::vector<std::string> mAddTrackSelectorList{};

   IDrawableModule* mDisplayModule{ nullptr };
   std::string mDisplayModuleContext{};
   ControlRecorder* mCurrentControlRecorder{ nullptr };
   std::vector<IUIControl*> mControls;
   bool mDisplayModuleIsShowingOverrideControls{ false };

   SessionOrganizer* mSessionOrganizer{ nullptr };
   PatchCableSource* mSessionOrganizerCable{ nullptr };
   int mTrackRowOffset{ 0 };
   int mSelectedTrackRow{ kTrackRowMixer };
   int mPreviousSelectedTrackRow{ -1 };
   double mDisplayModuleSelectTimeout{ 0.0 };
   int mSnapshotOffset{ 0 };
   std::array<bool, AbletonDevice::kMaxTrackButtons> mTrackButtonPressedForSnapshot{};
   std::array<bool, AbletonDevice::kMaxTrackButtons> mTrackButtonPressedForMute{};

   static constexpr int kNumPages{ 5 };
   std::array<PatchCableSource*, kNumPages> mGlobalControlModuleCables{};
   std::array<PatchCableSource*, kNumPages> mGlobalGridInterfaceCables{};
   int mGlobalModuleIndex{ 0 };
   float mGlobalModuleViewOffset{ 0.0f };
   float mModalModuleViewOffset{ 0.0f };

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
   std::array<double, AbletonDevice::kMaxMainEncoders> mLastTouchedKnobTime{};
   int mLastAdjustedKnobIndex{ -1 };
   double mLastAdjustedKnobTime{ -1 };
   double mLastResetTime{ -1 };
   int mLastGainAdjustTrackIndex{ -1 };
   double mLastGainAdjustTrackTime{ -1 };
   int mVolumeKnobTrackIndex{ -1 }; //track whose volume the volume knob last adjusted, on devices with page buttons
   double mVolumeKnobTrackTime{ -1 };
   float mTrackRowOffsetSmoothed{ 0 };
   bool mShowLCDOnScreen{ false };
   bool mAutoZoomToTrack{ false };
   bool mPageWithinModules{ false };
   bool mBottomRowMode{ false };
   bool mRequireTouchForKnobAdjustment{ false };
   double mLastClickyEncoderPressTime{ 0 };

   IAbletonGridController* mGridControlInterface{ nullptr };

   struct ControlState
   {
      ControlSurfaceLedState mQueuedLedState{};
      ControlSurfaceLedState mLedState{}; //what was last sent to the device
      LedPriority mLedPriority{ LedPriority::None };
      int mButtonState{ 0 };
      double mLastChangeTime{ 0.0 };
      double mPressTime{ 0.0 };
      bool mUsedSincePress{ false }; //has another control been used while this one was held?
   };

   std::array<ControlState, 128 * 2> mControlState{}; //bottom 128 are notes, top 128 are CCs, in the move layout

   MidiDevice mDevice;

   struct ControlInput
   {
      ControlInput() {}
      ControlInput(int _index, int _value)
      : index(_index)
      , value(_value)
      {}
      int index{ 0 }; //move layout index
      int value{ 0 };
   };

   LockFreeQueue<ControlInput> mQueuedInputs{};
   LockFreeQueue<MidiPitchBend> mQueuedPitchBendMessages{};
   LockFreeQueue<MidiPressure> mQueuedPressureMessages{};
};
