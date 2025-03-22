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
//  MidiController.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/14/13.
//
//

#pragma once

#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "RadioButton.h"
#include "ofxJSONElement.h"
#include "Transport.h"
#include "TextEntry.h"
#include "ModulationChain.h"
#include "INoteSource.h"

#define MIDI_PITCH_BEND_CONTROL_NUM 999
#define MIDI_PAGE_WIDTH 1000
#define MAX_MIDI_PAGES 32

enum MidiMessageType
{
   kMidiMessage_Note,
   kMidiMessage_Control,
   kMidiMessage_Program,
   kMidiMessage_PitchBend
};

enum ControlType
{
   kControlType_Slider,
   kControlType_SetValue,
   kControlType_Toggle,
   kControlType_Direct,
   kControlType_SetValueOnRelease,
   kControlType_Default
};

enum SpecialControlBinding
{
   kSpecialBinding_None,
   kSpecialBinding_Hover,
   kSpecialBinding_HotBind0,
   kSpecialBinding_HotBind1,
   kSpecialBinding_HotBind2,
   kSpecialBinding_HotBind3,
   kSpecialBinding_HotBind4,
   kSpecialBinding_HotBind5,
   kSpecialBinding_HotBind6,
   kSpecialBinding_HotBind7,
   kSpecialBinding_HotBind8,
   kSpecialBinding_HotBind9
};

enum class ChannelFilter
{
   kAny,
   k1,
   k2,
   k3,
   k4,
   k5,
   k6,
   k7,
   k8,
   k9,
   k10,
   k11,
   k12,
   k13,
   k14,
   k15,
   k16
};

namespace Json
{
   class Value;
}

class Monome;
class MidiController;
class GridControlTarget;
class INonstandardController;
class ScriptModule;

struct UIControlConnection
{
   UIControlConnection(MidiController* owner)
   : mUIOwner(owner)
   {}

   ~UIControlConnection();

   IUIControl* GetUIControl()
   {
      if (mSpecialBinding == kSpecialBinding_None)
         return mUIControl;
      else if (mSpecialBinding == kSpecialBinding_Hover)
         return gHoveredUIControl;
      int index = mSpecialBinding - kSpecialBinding_HotBind0;
      assert(index >= 0 && index <= 9);
      return gHotBindUIControl[index];
   }

   UIControlConnection* MakeCopy()
   {
      UIControlConnection* connection = new UIControlConnection(mUIOwner);
      connection->mMessageType = mMessageType;
      connection->mControl = mControl;
      connection->SetUIControl(mUIControl);
      connection->mType = mType;
      connection->mValue = mValue;
      connection->mLastControlValue = mLastControlValue;
      connection->mMidiOnValue = mMidiOnValue;
      connection->mMidiOffValue = mMidiOffValue;
      connection->mScaleOutput = mScaleOutput;
      connection->mBlink = mBlink;
      connection->mIncrementAmount = mIncrementAmount;
      connection->mTwoWay = mTwoWay;
      connection->mFeedbackControl = mFeedbackControl;
      connection->mSpecialBinding = mSpecialBinding;
      connection->mChannel = mChannel;
      connection->mPage = mPage;
      connection->mPageless = mPageless;
      connection->m14BitMode = m14BitMode;
      return connection;
   }

   void SetNext(UIControlConnection* next);

   void SetUIControl(IUIControl* control);
   void SetUIControl(std::string path);
   void CreateUIControls(int index);
   void Poll();
   void PreDraw();
   void DrawList(int index);
   void DrawLayout();
   void SetShowing(bool enabled);
   bool PostRepatch(PatchCableSource* cableSource, bool fromUserClick);

   int mControl{ -1 };
   IUIControl* mUIControl{ nullptr };
   ControlType mType{ ControlType::kControlType_Slider };
   MidiMessageType mMessageType{ MidiMessageType::kMidiMessage_Control };
   float mValue{ 0 };
   int mMidiOnValue{ 127 };
   int mMidiOffValue{ 0 };
   bool mScaleOutput{ false };
   bool mBlink{ false };
   float mIncrementAmount{ 0 };
   bool mTwoWay{ true };
   int mFeedbackControl{ -1 };
   SpecialControlBinding mSpecialBinding{ SpecialControlBinding::kSpecialBinding_None };
   int mChannel{ -1 };
   int mPage{ 0 };
   bool mPageless{ false };
   std::string mShouldRetryForUIControlAt{ "" };
   bool m14BitMode{ false };

   static bool sDrawCables;

   //state
   int mLastControlValue{ -1 };
   std::string mLastDisplayValue{ "" };
   double mLastActivityTime{ -9999 };
   MidiController* mUIOwner{ nullptr };

   //editor controls
   DropdownList* mMessageTypeDropdown{ nullptr };
   TextEntry* mControlEntry{ nullptr };
   DropdownList* mChannelDropdown{ nullptr };
   TextEntry* mUIControlPathEntry{ nullptr };
   char mUIControlPathInput[MAX_TEXTENTRY_LENGTH]{};
   DropdownList* mControlTypeDropdown{ nullptr };
   TextEntry* mValueEntry{ nullptr };
   TextEntry* mMidiOffEntry{ nullptr };
   TextEntry* mMidiOnEntry{ nullptr };
   Checkbox* mScaleOutputCheckbox{ nullptr };
   Checkbox* mBlinkCheckbox{ nullptr };
   Checkbox* m14BitModeCheckbox{ nullptr };
   TextEntry* mIncrementalEntry{ nullptr };
   Checkbox* mTwoWayCheckbox{ nullptr };
   DropdownList* mFeedbackDropdown{ nullptr };
   Checkbox* mPagelessCheckbox{ nullptr };
   ClickButton* mRemoveButton{ nullptr };
   ClickButton* mCopyButton{ nullptr };
   std::list<IUIControl*> mEditorControls;
};

enum ControlDrawType
{
   kDrawType_Button,
   kDrawType_Knob,
   kDrawType_Slider
};

struct ControlLayoutElement
{
   ControlLayoutElement()
   {}
   void Setup(MidiController* owner, MidiMessageType type, int control, ControlDrawType drawType, float incrementAmount, bool is14Bit, int offVal, int onVal, bool scaleOutput, ControlType connectionType, float x, float y, float w, float h);

   bool mActive{ false };
   MidiMessageType mType{ MidiMessageType::kMidiMessage_Control };
   int mControl{ 0 };
   ofVec2f mPosition;
   ofVec2f mDimensions;
   ControlDrawType mDrawType{ ControlDrawType::kDrawType_Slider };
   float mIncrementAmount{ 0 };
   int mOffVal{ 0 };
   int mOnVal{ 127 };
   bool mScaleOutput{ true };
   bool m14BitMode{ false };
   ControlType mConnectionType{ ControlType::kControlType_Slider };

   PatchCableSource* mControlCable{ nullptr };

   float mLastValue{ 0 };
   float mLastActivityTime{ -9999 };
};

struct GridLayout
{
   GridLayout()
   {}

   int mRows{ 1 };
   int mCols{ 8 };
   ofVec2f mPosition;
   ofVec2f mDimensions;
   MidiMessageType mType{ kMidiMessage_Note };
   std::vector<int> mControls;
   std::vector<int> mColors;

   PatchCableSource* mGridCable{ nullptr };
   GridControlTarget* mGridControlTarget[MAX_MIDI_PAGES]{};
};

#define NUM_LAYOUT_CONTROLS 128 + 128 + 128 + 1 + 1 //128 notes, 128 ccs, 128 program change, 1 pitch bend, 1 dummy

class MidiController : public MidiDeviceListener, public IDrawableModule, public IButtonListener, public IDropdownListener, public IRadioButtonListener, public IAudioPoller, public ITextEntryListener, public INoteSource, public IKeyboardFocusListener
{
public:
   MidiController();
   ~MidiController();
   static IDrawableModule* Create() { return new MidiController(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;

   void AddControlConnection(const ofxJSONElement& connection);
   UIControlConnection* AddControlConnection(MidiMessageType messageType, int control, int channel, IUIControl* uicontrol, int page = -1);
   void UseNegativeEdge(bool use) { mUseNegativeEdge = use; }
   void AddListener(MidiDeviceListener* listener, int page);
   void RemoveListener(MidiDeviceListener* listener);
   void AddScriptListener(ScriptModule* script);
   int GetPage() const { return mControllerPage; }
   bool IsInputConnected(bool immediate);
   std::string GetDeviceIn() const { return mDeviceIn; }
   std::string GetDeviceOut() const { return mDeviceOut; }
   UIControlConnection* GetConnectionForControl(MidiMessageType messageType, int control);
   UIControlConnection* GetConnectionForCableSource(const PatchCableSource* source);
   void ResyncControllerState();

   void SetVelocityMult(float mult) { mVelocityMult = mult; }
   void SetUseChannelAsVoice(bool use) { mUseChannelAsVoice = use; }
   void SetNoteOffset(int offset) { mNoteOffset = offset; }
   void SetPitchBendRange(float range) { mPitchBendRange = range; }

   void SendNote(int page, int pitch, int velocity, bool forceNoteOn = false, int channel = -1);
   void SendCC(int page, int ctl, int value, int channel = -1);
   void SendProgramChange(int page, int program, int channel = -1);
   void SendPitchBend(int page, int bend, int channel = -1);
   void SendData(int page, unsigned char a, unsigned char b, unsigned char c);
   void SendSysEx(int page, std::string data);

   INonstandardController* GetNonstandardController() { return mNonstandardController; }

   static std::vector<std::string> GetAvailableInputDevices();
   static std::vector<std::string> GetAvailableOutputDevices();

   //IDrawableModule
   void Poll() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void Exit() override;
   void KeyReleased(int key) override;

   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPressure(MidiPressure& pressure) override;
   void OnMidiProgramChange(MidiProgramChange& program) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;
   void OnMidi(const juce::MidiMessage& message) override;

   void OnTransportAdvanced(float amount) override;

   //IKeyboardFocusListener
   void OnKeyPressed(int key, bool isRepeat) override;
   bool ShouldConsumeKey(int key) override;
   bool CanTakeFocus() override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void TextEntryActivated(TextEntry* entry) override;
   void TextEntryComplete(TextEntry* entry) override;
   void PreRepatch(PatchCableSource* cableSource) override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   void OnCableGrabbed(PatchCableSource* cableSource) override;
   void SendControllerInfoString(int control, int type, std::string str);
   bool ShouldSendControllerInfoStrings() const { return mShouldSendControllerInfoStrings; }

   ControlLayoutElement& GetLayoutControl(int control, MidiMessageType type);

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   static std::string GetDefaultTooltip(MidiMessageType type, int control);

   static double sLastConnectedActivityTime;
   static IUIControl* sLastActivityUIControl;
   static double sLastBoundControlTime;
   static IUIControl* sLastBoundUIControl;

   bool IsEnabled() const override { return mEnabled; }

private:
   enum MappingDisplayMode
   {
      kHide,
      kList,
      kLayout
   };

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   void ConnectDevice();
   void MidiReceived(MidiMessageType messageType, int control, float scaledValue, int rawValue, int channel);
   void RemoveConnection(int control, MidiMessageType messageType, int channel, int page);
   int GetNumConnectionsOnPage(int page);
   void SetEntirePageToZero(int page);
   void BuildControllerList();
   void HighlightPageControls(int page);
   void OnDeviceChanged();
   int GetLayoutControlIndexForCable(PatchCableSource* cable) const;
   int GetLayoutControlIndexForMidi(MidiMessageType type, int control) const;
   std::string GetLayoutTooltip(int controlIndex);
   void UpdateControllerIndex();
   void LoadControllerLayout(std::string filename);
   bool JustBoundControl() const { return gTime - sLastBoundControlTime < 500; }

   const std::string kDefaultLayout = "default";

   float mVelocityMult{ 1 };
   bool mUseChannelAsVoice{ false };
   float mCurrentPitchBend{ 0 };
   int mNoteOffset{ 0 };
   float mPitchBendRange{ 2 };
   int mModwheelCC{ 1 }; // or 74 in Multidimensional Polyphonic Expression (MPE) spec
   float mModWheelOffset{ 0 };
   float mPressureOffset{ 0 };

   Modulations mModulation{ true };

   std::string mDeviceIn;
   std::string mDeviceOut;
   int mOutChannel{ 1 };
   MidiDevice mDevice;
   double mInitialConnectionTime{ 0 };
   ofxJSONElement mConnectionsJson;
   std::list<UIControlConnection*> mConnections;
   bool mSendCCOutput{ false };
   bool mUseNegativeEdge{ false }; // for midi toggle, accept on or off as a button press
   bool mSlidersDefaultToIncremental{ false };
   bool mBindMode{ false };
   Checkbox* mBindCheckbox{ nullptr };
   bool mTwoWay{ true };
   bool mSendTwoWayOnChange{ true };
   bool mResendFeedbackOnRelease{ false };
   ClickButton* mAddConnectionButton{ nullptr };
   std::list<MidiNote> mQueuedNotes;
   std::list<MidiControl> mQueuedControls;
   std::list<MidiProgramChange> mQueuedProgramChanges;
   std::list<MidiPitchBend> mQueuedPitchBends;
   DropdownList* mControllerList{ nullptr };
   Checkbox* mDrawCablesCheckbox{ nullptr };
   MappingDisplayMode mMappingDisplayMode{ MappingDisplayMode::kHide };
   RadioButton* mMappingDisplayModeSelector{ nullptr };
   int mLayoutFileIndex{ 0 };
   DropdownList* mLayoutFileDropdown{ nullptr };
   int mOscInPort{ 8000 };
   TextEntry* mOscInPortEntry{ nullptr };
   int mMonomeDeviceIndex{ -1 };
   DropdownList* mMonomeDeviceDropdown{ nullptr };
   bool mShouldSendControllerInfoStrings{ false };
   bool mSendSysex{ false };

   int mControllerIndex{ -1 };
   double mLastActivityTime{ -9999 };
   bool mLastActivityBound{ false };
   bool mShowActivityUIOverlay{ true };
   bool mBlink{ false };
   int mControllerPage{ 0 };
   DropdownList* mPageSelector{ nullptr };
   std::vector<std::list<MidiDeviceListener*> > mListeners;
   std::vector<ScriptModule*> mScriptListeners;
   bool mPrintInput{ false };
   std::string mLastInput;
   INonstandardController* mNonstandardController{ nullptr };
   bool mIsConnected{ false };
   bool mHasCreatedConnectionUIControls{ false };
   float mReconnectWaitTimer{ 0 };
   ChannelFilter mChannelFilter{ ChannelFilter::kAny };
   std::string mLastLoadedLayoutFile;
   ofxJSONElement mLayoutData;
   std::string mLayoutLoadError;

   std::array<ControlLayoutElement, NUM_LAYOUT_CONTROLS> mLayoutControls;
   int mHighlightedLayoutElement{ -1 };
   int mHoveredLayoutElement{ -1 };
   int mLayoutWidth{ 0 };
   int mLayoutHeight{ 0 };
   std::vector<GridLayout*> mGrids;

   ofMutex mQueuedMessageMutex;
};
