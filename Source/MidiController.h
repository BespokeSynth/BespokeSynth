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
   void Draw(int index);
   void SetShowing(bool enabled);
   void PostRepatch(PatchCableSource* cable);
   
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
   PatchCableSource* mControlCable;
   list<IUIControl*> mEditorControls;
};

class MidiController : public MidiDeviceListener, public IDrawableModule, public IButtonListener, public IDropdownListener, public IRadioButtonListener, public IAudioPoller, public ITextEntryListener
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
   void SetPage(int page);
   bool IsConnected();
   string GetDeviceIn() const { return mDeviceIn; }
   string GetDeviceOut() const { return mDeviceOut; }
   UIControlConnection* GetConnectionForControl(MidiMessageType messageType, int control);
   
   void SendNote(int page, int pitch, int velocity, bool forceNoteOn = false, int channel = -1);
   void SendCC(int page, int ctl, int value, int channel = -1);
   void SendData(int page, unsigned char a, unsigned char b, unsigned char c);

   //IDrawableModule
   void Poll() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void Exit() override;

   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiProgramChange(MidiProgramChange& program) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;
   
   void OnTransportAdvanced(float amount) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void TextEntryActivated(TextEntry* entry) override;
   void TextEntryComplete(TextEntry* entry) override;
   void PostRepatch(PatchCableSource* cable) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }

   void MidiReceived(MidiMessageType messageType, int control, float value, int channel = -1);
   void RemoveConnection(int control, MidiMessageType messageType, int channel, int page);
   void ResyncTwoWay();
   int GetNumConnectionsOnPage(int page);
   void SetEntirePageToZero(int page);
   void BuildControllerList();
   void HighlightPageControls(int page);

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
   ClickButton* mAddConnectionCheckbox;
   std::list<MidiNote> mQueuedNotes;
   std::list<MidiControl> mQueuedControls;
   std::list<MidiProgramChange> mQueuedProgramChanges;
   std::list<MidiPitchBend> mQueuedPitchBends;
   DropdownList* mControllerList;
   Checkbox* mDrawCablesCheckbox;

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
   
   ofMutex mMutex;
};

#endif /* defined(__modularSynth__MidiController__) */

