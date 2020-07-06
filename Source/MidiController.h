//
//  MidiController.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/14/13.
//
//

#ifndef __modularSynth__MidiController__
#define __modularSynth__MidiController__

#include <iostream>
#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "RadioButton.h"
#include "ofxJSONElement.h"
#include "Transport.h"
#include "INonstandardController.h"
#include "TextEntry.h"
#include "ModulationChain.h"
#include "INoteSource.h"

#define MIDI_PITCH_BEND_CONTROL_NUM 999
#define MIDI_PAGE_WIDTH 1000
#define MAX_MIDI_PAGES 10

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
   kControlType_SetValueOnRelease
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

namespace Json
{
   class Value;
}

class Monome;
class MidiController;
class GridController;

struct UIControlConnection
{
   UIControlConnection(MidiController* owner)
   {
      mMessageType = kMidiMessage_Control;
      mControl = -1;
      mUIControl = nullptr;
      mType = kControlType_Slider;
      mValue = 0;
      mLastControlValue = -1;
      mMidiOnValue = 127;
      mMidiOffValue = 0;
      mBlink = false;
      mIncrementAmount = 0;
      mTwoWay = true;
      mFeedbackControl = -1;
      mSpecialBinding = kSpecialBinding_None;
      mChannel = -1;
      mLastActivityTime = -9999;
      mUIControlPathInput[0] = 0;
      mPage = 0;
      mPageless = false;
      mUIOwner = owner;
   }
   
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
      connection->mUIControl = mUIControl;
      connection->mType = mType;
      connection->mValue = mValue;
      connection->mLastControlValue = mLastControlValue;
      connection->mMidiOnValue = mMidiOnValue;
      connection->mMidiOffValue = mMidiOffValue;
      connection->mBlink = mBlink;
      connection->mIncrementAmount = mIncrementAmount;
      connection->mTwoWay = mTwoWay;
      connection->mFeedbackControl = mFeedbackControl;
      connection->mSpecialBinding = mSpecialBinding;
      connection->mChannel = mChannel;
      connection->mPage = mPage;
      connection->mPageless = mPageless;
      return connection;
   }
   
   void SetNext(UIControlConnection* next);
   
   void SetUIControl(string path);
   void CreateUIControls(int index);
   void PreDraw();
   void DrawList(int index);
   void DrawLayout();
   void SetShowing(bool enabled);
   bool PostRepatch(PatchCableSource* cableSource, bool fromUserClick);
   
   int mControl;
   IUIControl* mUIControl;
   ControlType mType;
   MidiMessageType mMessageType;
   float mValue;
   int mMidiOnValue;
   int mMidiOffValue;
   bool mBlink;
   float mIncrementAmount;
   bool mTwoWay;
   int mFeedbackControl;
   SpecialControlBinding mSpecialBinding;
   int mChannel;
   int mPage;
   bool mPageless;
   
   static bool sDrawCables;
   
   //state
   int mLastControlValue;
   double mLastActivityTime;
   MidiController* mUIOwner;
   
   //editor controls
   DropdownList* mMessageTypeDropdown;
   TextEntry* mControlEntry;
   DropdownList* mChannelDropdown;
   TextEntry* mUIControlPathEntry;
   char mUIControlPathInput[MAX_TEXTENTRY_LENGTH];
   DropdownList* mControlTypeDropdown;
   TextEntry* mValueEntry;
   TextEntry* mMidiOffEntry;
   TextEntry* mMidiOnEntry;
   Checkbox* mBlinkCheckbox;
   TextEntry* mIncrementalEntry;
   Checkbox* mTwoWayCheckbox;
   DropdownList* mFeedbackDropdown;
   Checkbox* mPagelessCheckbox;
   ClickButton* mRemoveButton;
   ClickButton* mCopyButton;
   list<IUIControl*> mEditorControls;
};

enum ControlDrawType
{
   kDrawType_Button,
   kDrawType_Knob,
   kDrawType_Slider
};

struct ControlLayoutElement
{
   ControlLayoutElement() : mActive(false), mControlCable(nullptr) {}
   void Setup(MidiController* owner, MidiMessageType type, int control, ControlDrawType drawType, bool incremental, float x, float y, float w, float h);
   
   bool mActive;
   MidiMessageType mType;
   int mControl;
   ofVec2f mPosition;
   ofVec2f mDimensions;
   ControlDrawType mDrawType;
   bool mIncremental;
   
   PatchCableSource* mControlCable;
   
   float mLastValue;
   float mLastActivityTime;
};

struct GridLayout
{
   GridLayout() : mGridCable(nullptr) { for (int i=0; i<MAX_MIDI_PAGES; ++i) { mGridController[i] = nullptr; } }
   
   int mRows;
   int mCols;
   ofVec2f mPosition;
   ofVec2f mDimensions;
   MidiMessageType mType;
   vector<int> mControls;
   vector<int> mColors;
   
   PatchCableSource* mGridCable;
   GridController* mGridController[MAX_MIDI_PAGES];
};

#define NUM_LAYOUT_CONTROLS 128+128+1+1 //128 notes, 128 ccs, 1 pitch bend, 1 dummy

class MidiController : public MidiDeviceListener, public IDrawableModule, public IButtonListener, public IDropdownListener, public IRadioButtonListener, public IAudioPoller, public ITextEntryListener, public INoteSource
{
public:
   MidiController();
   ~MidiController();
   static IDrawableModule* Create() { return new MidiController(); }
   
   string GetTitleLabel() override { return Name() + string("   "); }
   void CreateUIControls() override;
   
   void Init() override;

   void AddControlConnection(const ofxJSONElement& connection);
   void AddControlConnection(MidiMessageType messageType, int control, int channel, IUIControl* uicontrol);
   void UseNegativeEdge(bool use) { mUseNegativeEdge = use; }
   void AddListener(MidiDeviceListener* listener, int page);
   void RemoveListener(MidiDeviceListener* listener);
   int GetPage() const { return mControllerPage; }
   bool IsInputConnected();
   string GetDeviceIn() const { return mDeviceIn; }
   string GetDeviceOut() const { return mDeviceOut; }
   UIControlConnection* GetConnectionForControl(MidiMessageType messageType, int control);
   void ConnectDevice();
   
   void SetVelocityMult(float mult) { mVelocityMult = mult; }
   void SetUseChannelAsVoice(bool use) { mUseChannelAsVoice = use; }
   void SetNoteOffset(int offset) { mNoteOffset = offset; }
   void SetPitchBendRange(float range) { mPitchBendRange = range; }
   
   void SendNote(int page, int pitch, int velocity, bool forceNoteOn = false, int channel = -1);
   void SendCC(int page, int ctl, int value, int channel = -1);
   void SendPitchBend(int page, int bend, int channel = -1);
   void SendData(int page, unsigned char a, unsigned char b, unsigned char c);

   //IDrawableModule
   void Poll() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void Exit() override;

   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPressure(MidiPressure& pressure) override;
   void OnMidiProgramChange(MidiProgramChange& program) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;
   void OnMidi(const MidiMessage& message) override;
   
   void OnTransportAdvanced(float amount) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void TextEntryActivated(TextEntry* entry) override;
   void TextEntryComplete(TextEntry* entry) override;
   void PreRepatch(PatchCableSource* cableSource) override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   void OnCableGrabbed(PatchCableSource* cableSource) override;
   
   ControlLayoutElement& GetLayoutControl(int control, MidiMessageType type);
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   enum MappingDisplayMode
   {
      kHide,
      kList,
      kLayout
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;

   void MidiReceived(MidiMessageType messageType, int control, float value, int channel = -1);
   void RemoveConnection(int control, MidiMessageType messageType, int channel, int page);
   void ResyncTwoWay();
   int GetNumConnectionsOnPage(int page);
   void SetEntirePageToZero(int page);
   void BuildControllerList();
   void HighlightPageControls(int page);
   void OnDeviceChanged();
   UIControlConnection* AddUIControlConnection();
   int GetLayoutControlIndexForCable(PatchCableSource* cable) const;
   
   float mVelocityMult;
   bool mUseChannelAsVoice;
   float mCurrentPitchBend;
   int mNoteOffset;
   float mPitchBendRange;
   int mModwheelCC;

   Modulations mModulation;

   string mDeviceIn;
   string mDeviceOut;
   int mOutChannel;
   MidiDevice mDevice;
   ofxJSONElement mConnectionsJson;
   list<UIControlConnection*> mConnections;
   bool mUseNegativeEdge;  //for midi toggle, accept on or off as a button press
   bool mSlidersDefaultToIncremental;
   bool mBindMode;
   Checkbox* mBindCheckbox;
   bool mTwoWay;
   ClickButton* mAddConnectionButton;
   std::list<MidiNote> mQueuedNotes;
   std::list<MidiControl> mQueuedControls;
   std::list<MidiProgramChange> mQueuedProgramChanges;
   std::list<MidiPitchBend> mQueuedPitchBends;
   DropdownList* mControllerList;
   Checkbox* mDrawCablesCheckbox;
   MappingDisplayMode mMappingDisplayMode;
   RadioButton* mMappingDisplayModeSelector;

   int mControllerIndex;
   double mLastActivityTime;
   bool mLastActivityBound;
   double mLastBindControllerTime;
   bool mBlink;
   int mControllerPage;
   DropdownList* mPageSelector;
   vector< list<MidiDeviceListener*> > mListeners;
   bool mPrintInput;
   string mLastInput;
   INonstandardController* mNonstandardController;
   bool mIsConnected;
   bool mHasCreatedConnectionUIControls;
   
   ControlLayoutElement mLayoutControls[NUM_LAYOUT_CONTROLS];
   int mHighlightedLayoutElement;
   int mLayoutWidth;
   int mLayoutHeight;
   list<GridLayout*> mGrids;
   
   ofMutex mQueuedMessageMutex;
};

#endif /* defined(__modularSynth__MidiController__) */

