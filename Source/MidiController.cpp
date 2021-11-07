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
//  MidiController.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/14/13.
//
//

#include "MidiController.h"
#include "IUIControl.h"
#include "SynthGlobals.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
//#include "Xbox360Controller.h"
#include "Transport.h"
#include "Monome.h"
#include "Profiler.h"
#include "OscController.h"
#include "PatchCableSource.h"
#include "GridController.h"
#include "MidiCapturer.h"
#include "ScriptModule.h"
#include "Push2Control.h"

using namespace juce;

bool UIControlConnection::sDrawCables = true;

namespace
{
   const int kLayoutControlsX = 5;
   const int kLayoutControlsY = 110;
   const int kLayoutButtonsX = 250;
   const int kLayoutButtonsY = 10;
}

MidiController::MidiController()
: mDevice(this)
, mUseNegativeEdge(false)
, mSlidersDefaultToIncremental(false)
, mBindMode(false)
, mBindCheckbox(nullptr)
, mTwoWay(true)
, mSendTwoWayOnChange(true)
, mResendFeedbackOnRelease(false)
, mControllerIndex(-1)
, mLastActivityTime(-9999)
, mLastConnectedActivityTime(-9999)
, mLastActivityUIControl(nullptr)
, mLastBoundControlTime(-9999)
, mLastBoundUIControl(nullptr)
, mBlink(false)
, mControllerPage(0)
, mPageSelector(nullptr)
, mPrintInput(false)
, mNonstandardController(nullptr)
, mControllerList(nullptr)
, mIsConnected(false)
, mHasCreatedConnectionUIControls(false)
, mReconnectWaitTimer(0)
, mChannelFilter(ChannelFilter::kAny)
, mVelocityMult(1)
, mUseChannelAsVoice(false)
, mNoteOffset(0)
, mCurrentPitchBend(0)
, mPitchBendRange(2)
, mModulation(true)
, mModwheelCC(1)  //or 74 in Multidimensional Polyphonic Expression (MPE) spec
, mMappingDisplayMode(kHide)
, mMappingDisplayModeSelector(nullptr)
, mOscInPort(8000)
, mMonomeDeviceIndex(-1)
, mHighlightedLayoutElement(-1)
, mHoveredLayoutElement(-1)
, mLayoutWidth(0)
, mLayoutHeight(0)
, mFoundLayoutFile(false)
{
   mListeners.resize(MAX_MIDI_PAGES);  
}

void MidiController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mControllerList = new DropdownList(this,"controller",3,2,&mControllerIndex, 157);
   mMappingDisplayModeSelector = new RadioButton(this,"mappingdisplay",mControllerList,kAnchor_Below,(int*)&mMappingDisplayMode, kRadioHorizontal);
   mBindCheckbox = new Checkbox(this,"bind (hold shift)",mMappingDisplayModeSelector,kAnchor_Below,&mBindMode);
   mPageSelector = new DropdownList(this,"page",mBindCheckbox,kAnchor_Right,&mControllerPage);
   mAddConnectionButton = new ClickButton(this,"add",12,300);
   mLayoutFileDropdown = new DropdownList(this, "layout", 3, 70, &mLayoutFileIndex);
   mOscInPortEntry = new TextEntry(this, "osc input port", 3, 88, 6, &mOscInPort, 0, 99999);
   mMonomeDeviceDropdown = new DropdownList(this, "monome", 3, 88, &mMonomeDeviceIndex);
   
   //mDrawCablesCheckbox = new Checkbox(this,"draw cables",200,26,&UIControlConnection::sDrawCables);
   
   for (int i=0; i<MAX_MIDI_PAGES; ++i)
      mPageSelector->AddLabel(("page "+ofToString(i)).c_str(), i);
   
   mMappingDisplayModeSelector->AddLabel("hide", kHide);
   mMappingDisplayModeSelector->AddLabel("layout", kLayout);
   mMappingDisplayModeSelector->AddLabel("list", kList);

   mLayoutFileDropdown->DrawLabel(true);
   mOscInPortEntry->DrawLabel(true);
   mMonomeDeviceDropdown->DrawLabel(true);
   
   mLayoutFileDropdown->AddLabel("default", 0);
   File dir(ofToDataPath("controllers"));
   Array<File> files;
   for (auto file : dir.findChildFiles(File::findFiles, false, "*.json"))
      files.add(file);
   files.sort();
   for (auto file : files)
      mLayoutFileDropdown->AddLabel(file.getFileName().toStdString(), mLayoutFileDropdown->GetNumValues());
}

MidiController::~MidiController()
{
   TheTransport->RemoveAudioPoller(this);
   delete mNonstandardController;
   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
      delete *i;
}

void MidiController::Init()
{
   IDrawableModule::Init();
   
   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
      delete *i;
   mConnections.clear();
   
   mHasCreatedConnectionUIControls = false;
   for (int i=0; i<mConnectionsJson.size(); ++i)
      AddControlConnection(mConnectionsJson[i]);

   TheTransport->AddAudioPoller(this);
}

void MidiController::AddListener(MidiDeviceListener* listener, int page)
{
   mListeners[page].push_back(listener);
   if (page == mControllerPage)
      listener->ControllerPageSelected();
}

void MidiController::RemoveListener(MidiDeviceListener* listener)
{
   for (int i=0; i<MAX_MIDI_PAGES; ++i)
      mListeners[i].remove(listener);
}

UIControlConnection* MidiController::AddControlConnection(MidiMessageType messageType, int control, int channel, IUIControl* uicontrol)
{
   RemoveConnection(control, messageType, channel, mControllerPage);

   UIControlConnection* connection = new UIControlConnection(this);
   connection->mMessageType = messageType;
   connection->mControl = control;
   connection->mUIControl = uicontrol;
   connection->mChannel = channel;
   connection->mPage = mControllerPage;
   if (dynamic_cast<Checkbox*>(uicontrol) != nullptr)
      connection->mType = kControlType_Toggle;
   else if (dynamic_cast<TextEntry*>(uicontrol) != nullptr)
      connection->mType = kControlType_Direct;
   else
      connection->mType = kControlType_Slider;
   if (mSlidersDefaultToIncremental)
      connection->mIncrementAmount = 1;

   int layoutControl = GetLayoutControlIndexForMidi(messageType, control);
   if (layoutControl != -1)
   {
      connection->mIncrementAmount = mLayoutControls[layoutControl].mIncremental ? 1 : 0;
      connection->mMidiOffValue = mLayoutControls[layoutControl].mOffVal;
      connection->mMidiOnValue = mLayoutControls[layoutControl].mOnVal;
      connection->mScaleOutput = mLayoutControls[layoutControl].mScaleOutput;
      if (mLayoutControls[layoutControl].mConnectionType != kControlType_Default)
         connection->mType = mLayoutControls[layoutControl].mConnectionType;
   }

   connection->CreateUIControls((int)mConnections.size());
   mConnections.push_back(connection);
   if (uicontrol != nullptr)
      uicontrol->AddRemoteController();

   return connection;
}

void MidiController::AddControlConnection(const ofxJSONElement& connection)
{
   try
   {
      if (connection.isMember("grid_index"))
      {
         int index = connection["grid_index"].asInt();
         if (index < (int)mGrids.size())
         {
            GridLayout* grid = mGrids[index];
            for (int page = 0; page < connection["grid_pages"].size(); ++page)
            {
               std::string path = connection["grid_pages"][page].asString();
               if (path.length() > 0)
                  grid->mGridControlTarget[page] = dynamic_cast<GridControlTarget*>(GetOwningContainer()->FindUIControl(path));
               else
                  grid->mGridControlTarget[page] = nullptr;
            }
         }
         return;
      }

      int control = connection["control"].asInt() % MIDI_PAGE_WIDTH;
      std::string path = connection["uicontrol"].asString();
      std::string type = connection["type"].asString();
      MidiMessageType msgType = kMidiMessage_Control;
      if (type == "control")
      {
         msgType = kMidiMessage_Control;
      }
      else if (type == "note")
      {
         msgType = kMidiMessage_Note;
      }
      else if (type == "program")
      {
         msgType = kMidiMessage_Program;
      }
      else if (type == "pitchbend")
      {
         msgType = kMidiMessage_PitchBend;
         control = MIDI_PITCH_BEND_CONTROL_NUM;
      }
      int channel = -1;
      if (!connection["channel"].isNull())
         channel = connection["channel"].asInt();

      int page = -1;
      bool pageless = false;
      if (!connection["page"].isNull())
         page = connection["page"].asInt();
      if (page == -1)
         pageless = true;

      UIControlConnection* controlConnection = new UIControlConnection(this);
      controlConnection->mMessageType = msgType;
      controlConnection->mControl = control;
      controlConnection->SetUIControl(path);

      ControlType controlType = kControlType_SetValue;
      if (connection["toggle"].asBool())
         controlType = kControlType_Toggle;
      else if (connection["direct"].asBool())
         controlType = kControlType_Direct;
      else if (connection["value"].isNull())
         controlType = kControlType_Slider;
      else if (connection["release"].asBool())
         controlType = kControlType_SetValueOnRelease;
      controlConnection->mType = controlType;

      controlConnection->mChannel = channel;
      controlConnection->mPage = page;
      controlConnection->mPageless = pageless;

      if (!connection["midi_on_value"].isNull())
         controlConnection->mMidiOnValue = connection["midi_on_value"].asInt();

      if (!connection["midi_off_value"].isNull())
         controlConnection->mMidiOffValue = connection["midi_off_value"].asInt();

      if (!connection["blink"].isNull())
         controlConnection->mBlink = connection["blink"].asBool();

      if (!connection["value"].isNull())
         controlConnection->mValue = connection["value"].asDouble();

      if (!connection["increment_amount"].isNull())
         controlConnection->mIncrementAmount = connection["increment_amount"].asDouble();

      if (!connection["twoway"].isNull())
         controlConnection->mTwoWay = connection["twoway"].asBool();

      if (!connection["feedbackcontrol"].isNull())
         controlConnection->mFeedbackControl = connection["feedbackcontrol"].asInt();

      //controlConnection->CreateUIControls(this, mConnections.size()); //do this on the first draw instead, to avoid a long init time when setting up a bunch of minimized controllers
      mConnections.push_back(controlConnection);

      if (!connection["pages"].isNull())
      {
         for (int i = 0; i < connection["pages"].size(); ++i)
         {
            IUIControl* uicontrolNextPage = TheSynth->FindUIControl(connection["pages"][i].asCString());
            if (uicontrolNextPage)
            {
               UIControlConnection* nextPageConnection = new UIControlConnection(*controlConnection);
               nextPageConnection->mPage += i + 1;
               nextPageConnection->mUIControl = uicontrolNextPage;
               nextPageConnection->mEditorControls.clear(); //TODO(Ryan) temp fix
               nextPageConnection->CreateUIControls((int)mConnections.size());
               mConnections.push_back(nextPageConnection);
               uicontrolNextPage->AddRemoteController();
            }
         }
      }
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }
}

void MidiController::OnTransportAdvanced(float amount)
{
   PROFILER(MidiController);
   
   mQueuedMessageMutex.lock();
   
   double firstNoteTimestampMs = -1;
   for (auto note = mQueuedNotes.begin(); note != mQueuedNotes.end(); ++note)
   {
      int voiceIdx = -1;
      
      if (mUseChannelAsVoice)
         voiceIdx = note->mChannel - 1;
      
      //TODO(Ryan) how can I use note->mTimestamp to get more accurate timing for midi input?
      //this here is not accurate, but prevents notes played within the same buffer from having the exact same time
      double playTime;
      if (firstNoteTimestampMs == -1) //this is the first note
      {
         firstNoteTimestampMs = note->mTimestampMs;
         playTime = gTime;
      }
      else
      {
         playTime = gTime + (note->mTimestampMs - firstNoteTimestampMs);
      }
      PlayNoteOutput(playTime, note->mPitch + mNoteOffset, MIN(127,note->mVelocity*mVelocityMult), voiceIdx, ModulationParameters(mModulation.GetPitchBend(voiceIdx), mModulation.GetModWheel(voiceIdx), mModulation.GetPressure(voiceIdx), 0));
      
      for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
         (*i)->OnMidiNote(*note);
   }
   mQueuedNotes.clear();
   
   for (auto note = mQueuedControls.begin(); note != mQueuedControls.end(); ++note)
   {
      for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
         (*i)->OnMidiControl(*note);
   }
   mQueuedControls.clear();
   
   for (auto note = mQueuedProgramChanges.begin(); note != mQueuedProgramChanges.end(); ++note)
   {
      for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
         (*i)->OnMidiProgramChange(*note);
   }
   mQueuedProgramChanges.clear();
   
   for (auto note = mQueuedPitchBends.begin(); note != mQueuedPitchBends.end(); ++note)
   {
      for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
         (*i)->OnMidiPitchBend(*note);
   }
   mQueuedPitchBends.clear();
   
   mQueuedMessageMutex.unlock();
}

void MidiController::OnMidiNote(MidiNote& note)
{
   if (!mEnabled || (mChannelFilter != ChannelFilter::kAny && note.mChannel != (int)mChannelFilter))
      return;

   if (mUseChannelAsVoice && note.mVelocity > 0)
   {
      int voiceIdx = note.mChannel - 1;
      mModulation.GetPitchBend(voiceIdx)->SetValue(0);
   }
   
   MidiReceived(kMidiMessage_Note, note.mPitch, note.mVelocity/127.0f, note.mChannel);
   
   mQueuedMessageMutex.lock();
   mQueuedNotes.push_back(note);
   mQueuedMessageMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " note: " << note.mPitch << ", " << note.mVelocity;
}

void MidiController::OnMidiControl(MidiControl& control)
{
   if (!mEnabled || (mChannelFilter != ChannelFilter::kAny && control.mChannel != (int)mChannelFilter))
      return;
   
   int voiceIdx = -1;
   
   if (mUseChannelAsVoice)
      voiceIdx = control.mChannel - 1;
   
   if (control.mControl == mModwheelCC)
   {
      //if (mModwheelCC == 74) //MPE
      //   mModulation.GetModWheel(voiceIdx)->SetValue((control.mValue-63) / 127.0f * 2);
      //else
      mModulation.GetModWheel(voiceIdx)->SetValue(control.mValue / 127.0f);
   }
   
   MidiReceived(kMidiMessage_Control, control.mControl, control.mValue/127.0f, control.mChannel);
   
   mQueuedMessageMutex.lock();
   mQueuedControls.push_back(control);
   mQueuedMessageMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " control: " << control.mControl << ", " << control.mValue;
}

void MidiController::OnMidiPressure(MidiPressure& pressure)
{
   if (!mEnabled || (mChannelFilter != ChannelFilter::kAny && pressure.mChannel != (int)mChannelFilter))
      return;
   
   int voiceIdx = -1;
   
   if (mUseChannelAsVoice)
      voiceIdx = pressure.mChannel - 1;
   
   mModulation.GetPressure(voiceIdx)->SetValue(pressure.mPressure / 127.0f);
   
   mNoteOutput.SendPressure(pressure.mPitch, pressure.mPressure);
}

void MidiController::OnMidiProgramChange(MidiProgramChange& program)
{
   if (!mEnabled || (mChannelFilter != ChannelFilter::kAny && program.mChannel != (int)mChannelFilter))
      return;
   
   MidiReceived(kMidiMessage_Program, program.mProgram, 1, program.mChannel);
   
   mQueuedMessageMutex.lock();
   mQueuedProgramChanges.push_back(program);
   mQueuedMessageMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " program change: " << program.mProgram;
}

void MidiController::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (!mEnabled || (mChannelFilter != ChannelFilter::kAny && pitchBend.mChannel != (int)mChannelFilter))
      return;
   
   int voiceIdx = -1;
   
   float amount = (pitchBend.mValue - 8192.0f) / (8192.0f/mPitchBendRange);
   
   if (mUseChannelAsVoice)
      voiceIdx = pitchBend.mChannel - 1;
   else
      mCurrentPitchBend = amount;
   
   mModulation.GetPitchBend(voiceIdx)->SetValue(amount);
   
   MidiReceived(kMidiMessage_PitchBend, MIDI_PITCH_BEND_CONTROL_NUM, pitchBend.mValue/16383.0f, pitchBend.mChannel);   //16383 = max pitch bend
 
   mQueuedMessageMutex.lock();
   mQueuedPitchBends.push_back(pitchBend);
   mQueuedMessageMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " pitch bend: " << pitchBend.mValue;
}

void MidiController::OnMidi(const MidiMessage& message)
{
   if (!mEnabled || (mChannelFilter != ChannelFilter::kAny && message.getChannel() != (int)mChannelFilter && !message.isSysEx()))
      return;
   mNoteOutput.SendMidi(message);
}

void MidiController::MidiReceived(MidiMessageType messageType, int control, float value, int channel)
{
   assert(mEnabled);
   
   mLastActivityBound = false;
   //if (value > 0)
      mLastActivityTime = gTime;
   
   GetLayoutControl(control, messageType).mLastActivityTime = gTime;
   GetLayoutControl(control, messageType).mLastValue = value;
   
   if (JustBoundControl())   //no midi messages if we just bound something, to avoid changing that thing we just bound
      return;
   
   if (messageType == kMidiMessage_Control)
      mLastInput = "cc ";
   if (messageType == kMidiMessage_Note)
      mLastInput = "note ";
   if (messageType == kMidiMessage_Program)
      mLastInput = "program change ";
   if (messageType == kMidiMessage_PitchBend)
      mLastInput = "pitchbend";
   
   if (messageType != kMidiMessage_PitchBend)
      mLastInput += ofToString(control);
   
   mLastInput += ", value: " + ofToString(value,2) + ", channel: " + ofToString(channel);

   if (mBindMode && gBindToUIControl)
   {
      AddControlConnection(messageType, control, channel, gBindToUIControl);
      mLastBoundControlTime = gTime;
      mLastBoundUIControl = gBindToUIControl;
      mLastBoundUIControl->StartBeacon();
      gBindToUIControl = nullptr;
      return;
   }

   if (Push2Control::sBindToUIControl)
   {
      AddControlConnection(messageType, control, channel, Push2Control::sBindToUIControl);
      mLastBoundControlTime = gTime;
      mLastBoundUIControl = Push2Control::sBindToUIControl;
      mLastBoundUIControl->StartBeacon();
      Push2Control::sBindToUIControl = nullptr;
      return;
   }

   if (mBindMode && gHoveredUIControl && (GetKeyModifiers() == kModifier_Shift))
   {
      AddControlConnection(messageType, control, channel, gHoveredUIControl);
      mLastBoundControlTime = gTime;
      mLastBoundUIControl = gHoveredUIControl;
      mLastBoundUIControl->StartBeacon();
      return;
   }

   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
   {
      UIControlConnection* connection = *i;
      if (connection->mMessageType == messageType &&
          (connection->mControl == control || messageType == kMidiMessage_PitchBend) &&
          (connection->mPageless || connection->mPage == mControllerPage) &&
          (connection->mChannel == -1 || connection->mChannel == channel))
      {
         mLastActivityBound = true;
         //if (value > 0)
            connection->mLastActivityTime = gTime;
         
         IUIControl* uicontrol = connection->GetUIControl();
         if (uicontrol == nullptr)
            continue;
         
         mLastActivityUIControl = uicontrol;
         mLastConnectedActivityTime = gTime;

         if (connection->mType == kControlType_Slider)
         {
            if (connection->mIncrementAmount != 0)
            {
               float curValue = uicontrol->GetMidiValue();
               float increment = connection->mIncrementAmount / 100;
               if (GetKeyModifiers() & kModifier_Shift)
                  increment /= 50;
               const float midpoint = 64.0f / 127.0f;
               if (value != midpoint)
               {
                   if (value > midpoint)
                      curValue += increment;
                   else
                      curValue -= increment;
                   uicontrol->SetFromMidiCC(curValue);
               }
            }
            else
            {
               if (connection->mMessageType == kMidiMessage_Note)
                  value = value>0 ? 1 : 0;
               if (connection->mScaleOutput && (connection->mMidiOffValue != 0 || connection->mMidiOnValue != 127))
                  value = ofLerp(connection->mMidiOffValue/127.0f, connection->mMidiOnValue/127.0f, value);
               uicontrol->SetFromMidiCC(value);
            }
            uicontrol->StartBeacon();
         }
         else if (connection->mType == kControlType_Toggle)
         {
            if (value > 0)
            {
               float val = uicontrol->GetMidiValue();
               uicontrol->SetValue(val == 0);
               uicontrol->StartBeacon();
            }
         }
         else if (connection->mType == kControlType_SetValue)
         {
            if (value > 0 || mUseNegativeEdge)
            {
               if (connection->mIncrementAmount != 0)
                  uicontrol->Increment(connection->mIncrementAmount);
               else
                  uicontrol->SetValue(connection->mValue);
               uicontrol->StartBeacon();
            }
         }
         else if (connection->mType == kControlType_SetValueOnRelease)
         {
            if (value == 0)
            {
               if (connection->mIncrementAmount != 0)
                  uicontrol->Increment(connection->mIncrementAmount);
               else
                  uicontrol->SetValue(connection->mValue);
               uicontrol->StartBeacon();
            }
         }
         else if (connection->mType == kControlType_Direct)
         {
            uicontrol->SetValue(value*127);
            uicontrol->StartBeacon();
         }

         if (!mSendTwoWayOnChange)
            connection->mLastControlValue = int(uicontrol->GetMidiValue() * 127);   //set expected value here, so we don't send the value. otherwise, this will send the input value right back as output. (although, this behavior is desirable for some controllers, hence mSendTwoWayOnChange)

         if (mResendFeedbackOnRelease && value == 0)
            connection->mLastControlValue = -999;  //force feedback update on release
      }
   }
   
   for (auto* grid : mGrids)
   {
      if (grid->mType == messageType && grid->mGridControlTarget[mControllerPage] != nullptr && grid->mGridControlTarget[mControllerPage]->GetGridController() != nullptr)
      {
         for (int i=0; i<grid->mControls.size(); ++i)
         {
            if (grid->mControls[i] == control)
            {
               dynamic_cast<GridControllerMidi*>(grid->mGridControlTarget[mControllerPage]->GetGridController())->OnInput(control, value);
               grid->mGridCable->AddHistoryEvent(gTime, grid->mGridControlTarget[mControllerPage]->GetGridController()->HasInput());
               break;
            }
         }
      }
   }
   
   /*if (mLastActivityBound == false && mTwoWay) //if this didn't affect anything, give that feedback on the controller by keeping it at zero
   {
      //if (messageType == kMidiMessage_Note)
      //   SendNote(control, 0);
      if (messageType == kMidiMessage_Control)
         SendCC(mControllerPage, control, 0);
   }*/

   for (auto* script : mScriptListeners)
      script->MidiReceived(messageType, control, value, channel);
}

void MidiController::AddScriptListener(ScriptModule* script)
{
   if (!VectorContains(script, mScriptListeners))
      mScriptListeners.push_back(script); 
}

void MidiController::RemoveConnection(int control, MidiMessageType messageType, int channel, int page)
{
   IUIControl* removed = nullptr;
   
   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
   {
      if ((*i)->mControl == control && (*i)->mMessageType == messageType && (*i)->mChannel == channel && ((*i)->mPage == page || (*i)->mPageless))
      {
         removed = (*i)->mUIControl;
         delete *i;
         i = mConnections.erase(i);
         break;
      }
   }

   if (removed)
   {
      bool remotelyControlled = false;
      for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
      {
         if ((*i)->mUIControl == removed)
         {
            remotelyControlled = true;
            break;
         }
      }

      if (!remotelyControlled)
         removed->RemoveRemoteController();
   }
}

void MidiController::Poll()
{
   bool lastBlink = mBlink;
   mBlink = int(TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop() * 2) % 2 == 0;
   
   if (IsInputConnected(!K(immediate)) || mReconnectWaitTimer > 0)
   {
      if (!mIsConnected && gTime - mInitialConnectionTime > 1000)
      {
         if (mNonstandardController)
         {
            if (mNonstandardController->Reconnect())
            {
               ResyncTwoWay();
               mIsConnected = true;

               for (auto* grid : mGrids)
               {
                  if (grid->mGridControlTarget[mControllerPage] != nullptr && grid->mGridControlTarget[mControllerPage]->GetGridController() != nullptr)
                     dynamic_cast<GridControllerMidi*>(grid->mGridControlTarget[mControllerPage]->GetGridController())->OnControllerPageSelected();
               }
            }
         }
         else
         {
            if (mReconnectWaitTimer <= 0)
            {
               mReconnectWaitTimer = 1000;
               mDevice.DisconnectInput();
               mDevice.DisconnectOutput();
            }
            else
            {
               mReconnectWaitTimer -= 1/(ofGetFrameRate()) * 1000;
               if (mReconnectWaitTimer <= 0)
                  ConnectDevice();
            }
         }

         if (mIsConnected)
         {
            for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
               (*i)->ControllerPageSelected();
         }
      }
   }
   else
   {
      if (mIsConnected)
      {
         mDevice.DisconnectInput();
         mDevice.DisconnectOutput();
         mIsConnected = false;
      }
   }
   
   if (mTwoWay)
   {
      for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
      {
         UIControlConnection* connection = *i;
         
         if (connection->mTwoWay == false)
            continue;
         
         IUIControl* uicontrol = connection->GetUIControl();
         if (uicontrol == nullptr)
            continue;

         if (JustBoundControl() && uicontrol == mLastBoundUIControl)
            continue;
         
         if (!connection->mPageless && connection->mPage != mControllerPage)
            continue;
         
         int control = connection->mControl;
         int messageType = connection->mMessageType;
         
         if (connection->mFeedbackControl != -1) // "self"
         {
            control = connection->mFeedbackControl%128;
            if (connection->mFeedbackControl < 128)
               messageType = kMidiMessage_Control;
            else
               messageType = kMidiMessage_Note;
         }
         
         if (connection->mFeedbackControl == -2) // "none"
            continue;
         
         int curValue = int(uicontrol->GetMidiValue() * 127);
         if (curValue != connection->mLastControlValue ||
             (connection->mBlink && lastBlink != mBlink))
         {
            if (connection->mType == kControlType_Toggle)
            {
               int outVal = curValue;
               if (curValue && (connection->mMidiOnValue != 127 || connection->mBlink))
               {
                  if (connection->mBlink == false)
                     outVal = connection->mMidiOnValue;
                  else
                     outVal = mBlink ? connection->mMidiOnValue : connection->mMidiOffValue;
               }
               else if (!curValue && connection->mMidiOffValue != 0)
               {
                  outVal = connection->mMidiOffValue;
               }
               if (messageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, outVal, true, connection->mChannel);
               else if (messageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, outVal, connection->mChannel);
               else if (messageType == kMidiMessage_PitchBend)
                  SendPitchBend(mControllerPage, outVal, connection->mChannel);
            }
            else if (connection->mType == kControlType_Slider)
            {
               int outVal = curValue;
               if (connection->mMidiOnValue != 127 ||
                   connection->mMidiOffValue != 0) //uses defined slider range for output
               {
                  outVal = int((curValue / 127.0f) * (connection->mMidiOnValue - connection->mMidiOffValue) + connection->mMidiOffValue);
               }
               if (messageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, outVal, true, connection->mChannel);
               else if (messageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, outVal, connection->mChannel);
               else if (messageType == kMidiMessage_PitchBend)
                  SendPitchBend(mControllerPage, outVal, connection->mChannel);
            }
            else if (connection->mType == kControlType_SetValue)
            {
               float realValue = uicontrol->GetValue();
               bool valuesAreEqual = fabsf(realValue - connection->mValue) < .0001f;
               int outVal = 0;
               if ((!uicontrol->IsBitmask() && valuesAreEqual) ||
                   (uicontrol->IsBitmask() && ((int)realValue & (1 << (int)connection->mValue))))
               {
                  if (connection->mBlink == false)
                     outVal = connection->mMidiOnValue;
                  else
                     outVal = mBlink ? connection->mMidiOnValue : connection->mMidiOffValue;
               }
               else
               {
                  outVal = connection->mMidiOffValue;
               }
               if (messageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, outVal, true, connection->mChannel);
               else if (messageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, outVal, connection->mChannel);
               else if (messageType == kMidiMessage_PitchBend)
                  SendPitchBend(mControllerPage, outVal, connection->mChannel);
            }
            else if (connection->mType == kControlType_Direct)
            {
               curValue = uicontrol->GetValue();
               if (messageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, uicontrol->GetValue(), true, connection->mChannel);
               else if (messageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, uicontrol->GetValue(), connection->mChannel);
               else if (messageType == kMidiMessage_PitchBend)
                  SendPitchBend(mControllerPage, uicontrol->GetValue(), connection->mChannel);
            }
            connection->mLastControlValue = curValue;
         }
      }
   }
   
   if (mNonstandardController != nullptr)
      mNonstandardController->Poll();
}

void MidiController::Exit()
{
   IDrawableModule::Exit();
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (connection->mMessageType == kMidiMessage_Control)
         mDevice.SendCC(connection->mControl, 0, connection->mChannel);
      if (connection->mMessageType == kMidiMessage_Note)
         mDevice.SendNote(gTime, connection->mControl, 0, false, connection->mChannel);
   }
}

void MidiController::DrawModule()
{
   if (gTime - mLastActivityTime > 0 && gTime - mLastActivityTime < 200)
   {
      ofPushStyle();
      if (mLastActivityBound)
         ofSetColor(0,255,0,255*(1-(gTime - mLastActivityTime)/200));
      else
         ofSetColor(255,0,0,255*(1-(gTime - mLastActivityTime)/200));
      ofFill();
      ofRect(30+gFont.GetStringWidth(Name(), 15),-11,10,10);
      ofPopStyle();
   }
   
   if (!mIsConnected)
   {
      float xStart = 30+gFont.GetStringWidth(Name(), 15);
      float yStart = -11;
      
      ofPushStyle();
      ofSetColor(ofColor::red);
      ofLine(xStart,yStart,xStart+10,yStart+10);
      ofLine(xStart+10,yStart,xStart,yStart+10);
      ofPopStyle();
   }
   
   if (TheSynth->GetLastClickedModule() == this)
   {
      for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
         DrawConnection(dynamic_cast<IDrawableModule*>(*i));
   }

   if (Minimized() || IsVisible() == false)
      return;
   
   if (!mHasCreatedConnectionUIControls)
   {
      int i=0;
      for (UIControlConnection* connection : mConnections)
      {
         connection->CreateUIControls(i);
         ++i;
      }
      mHasCreatedConnectionUIControls = true;
   }
   
   mControllerList->Draw();
   mMappingDisplayModeSelector->Draw();
   mBindCheckbox->Draw();
   mPageSelector->Draw();
   //mDrawCablesCheckbox->Draw();
   mLayoutFileDropdown->SetShowing(mMappingDisplayMode == kLayout);
   mLayoutFileDropdown->Draw();
   mOscInPortEntry->SetShowing(mDeviceIn == "osccontroller" && mMappingDisplayMode == kLayout);
   mOscInPortEntry->Draw();
   mMonomeDeviceDropdown->SetShowing(mDeviceIn == "monome" && mMappingDisplayMode == kLayout);
   mMonomeDeviceDropdown->Draw();
   
   for (int i=0; i<NUM_LAYOUT_CONTROLS; ++i)
   {
      if (mLayoutControls[i].mControlCable)
         mLayoutControls[i].mControlCable->SetEnabled(false);
   }
   for (auto* grid : mGrids)
   {
      if (grid->mGridCable)
         grid->mGridCable->SetEnabled(mMappingDisplayMode == kLayout);
   }
   
   if (mMappingDisplayMode == kHide)
   {
      for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
      {
         UIControlConnection* connection = *iter;
         connection->SetShowing(false);   //set all to not be showing
      }
   }
   else
   {
      float w,h;
      GetDimensions(w, h);
      
      DrawTextNormal("last input: "+mLastInput,60,h-5);
      
      if (gTime - mLastActivityTime > 0 && gTime - mLastActivityTime < 200)
      {
         ofPushStyle();
         if (mLastActivityBound)
            ofSetColor(0,255,0,255*(1-(gTime - mLastActivityTime)/200));
         else
            ofSetColor(255,0,0,255*(1-(gTime - mLastActivityTime)/200));
         ofFill();
         ofRect(48,h-14,10,10);
         ofPopStyle();
      }
      
      mPageSelector->SetShowing(true);
   }
   
   if (mMappingDisplayMode == kLayout)
   {
      ofPushStyle();
      if (mReconnectWaitTimer > 0)
      {
         ofSetColor(ofColor::yellow);
         DrawTextNormal("reconnecting...", 3, 63);
      }
      else if (mIsConnected)
      {
         ofSetColor(ofColor::green);
         DrawTextNormal("connected", 3, 63);
      }
      else
      {
         ofSetColor(ofColor::red);
         DrawTextNormal("not connected", 3, 63);
      }
      ofPopStyle();
      
      ofPushStyle();
      ofNoFill();
      for (int i=0; i<NUM_LAYOUT_CONTROLS; ++i)
      {
         ControlLayoutElement& control = mLayoutControls[i];
         if (control.mActive)
         {
            ofVec2f center(control.mPosition.x + control.mDimensions.x/2, control.mPosition.y + control.mDimensions.y/2);
            float uiControlValue = 0;
            
            if (control.mControlCable)
            {
               control.mControlCable->SetEnabled(UIControlConnection::sDrawCables);
               control.mControlCable->SetManualPosition(center.x, center.y);
               control.mControlCable->SetPatchCableDrawMode(kPatchCableDrawMode_SourceOnHoverOnly);
               
               UIControlConnection* connection = GetConnectionForControl(control.mType, control.mControl);
               if (connection)
               {
                  control.mControlCable->SetTarget(connection->mUIControl);
                  if (connection->mUIControl)
                  {
                     uiControlValue = connection->mUIControl->GetMidiValue();
                     control.mControlCable->GetPatchCables()[0]->SetUIControlConnection(connection);
                  }
               }
               else
               {
                  if (PatchCable::sActivePatchCable == nullptr)
                     control.mControlCable->ClearPatchCables();
               }
            }
            
            if (gTime - control.mLastActivityTime < 200)
            {
               ofPushStyle();
               if (control.mControlCable && control.mControlCable->GetTarget())
                  ofSetColor(0,255,0,255*(1-(gTime - control.mLastActivityTime)/200));
               else
                  ofSetColor(255,0,0,255*(1-(gTime - control.mLastActivityTime)/200));
               ofFill();
               ofRect(control.mPosition.x,control.mPosition.y,5,5);
               ofPopStyle();
            }
            
            ofPushStyle();
            
            if (mHighlightedLayoutElement == i)
            {
               ofSetColor(255, 255, 255, gModuleDrawAlpha);
            }
            else
            {
               if (control.mLastActivityTime > 0)
                  ofSetColor(IDrawableModule::GetColor(GetModuleType()), gModuleDrawAlpha);
               else
                  ofSetColor(IDrawableModule::GetColor(GetModuleType()), gModuleDrawAlpha * .3f);
            }
            
            if (control.mDrawType == kDrawType_Button)
            {
               ofRect(control.mPosition.x, control.mPosition.y, control.mDimensions.x, control.mDimensions.y, 4);
               
               bool on = control.mLastValue > 0;
               if (control.mType == kMidiMessage_Program)
                  on = control.mLastValue > 0 && (gTime - control.mLastActivityTime < 250);

               if (on)
               {
                  float fadeAmount = ofClamp(ofLerp(.5f, 1, control.mLastValue), 0, 1);
                  ofPushStyle();
                  ofFill();
                  ofSetColor(255 * fadeAmount, 255 * fadeAmount, 255 * fadeAmount, gModuleDrawAlpha);
                  ofRect(control.mPosition.x, control.mPosition.y, control.mDimensions.x, control.mDimensions.y, 4);
                  ofPopStyle();
               }
            }
            
            if (control.mDrawType == kDrawType_Knob)
            {
               float value = control.mLastValue;
               if (control.mIncremental)
                  value = uiControlValue;
               
               ofCircle(center.x, center.y, control.mDimensions.x/2);
               ofPushStyle();
               ofSetColor(255, 255, 255, gModuleDrawAlpha);
               float angle = ofLerp(.1f, .9f, value) * FTWO_PI;
               ofLine(center.x, center.y, center.x - sinf(angle) * control.mDimensions.x/2, center.y + cosf(angle) * control.mDimensions.x/2);
               ofPopStyle();
            }
               
            if (control.mDrawType == kDrawType_Slider)
            {
               float value = control.mLastValue;
               if (control.mIncremental)
                  value = uiControlValue;
               
               ofRect(control.mPosition.x, control.mPosition.y, control.mDimensions.x, control.mDimensions.y, 0);
               ofPushStyle();
               ofSetColor(255, 255, 255, gModuleDrawAlpha);
               ofFill();
               if (control.mDimensions.x > control.mDimensions.y)
                  ofLine(control.mPosition.x+value*control.mDimensions.x,
                         control.mPosition.y,
                         control.mPosition.x+value*control.mDimensions.x,
                         control.mPosition.y+control.mDimensions.y);
               else
                  ofLine(control.mPosition.x,
                         control.mPosition.y+(1-value)*control.mDimensions.y,
                         control.mPosition.x+control.mDimensions.x,
                         control.mPosition.y+(1-value)*control.mDimensions.y);
               ofPopStyle();
            }
            
            ofPopStyle();
         }
      }
      
      for (auto* grid : mGrids)
         ofRect(grid->mPosition.x, grid->mPosition.y, grid->mDimensions.x, grid->mDimensions.y);
      
      ofPopStyle();
      
      for (auto connection : mConnections)
         connection->SetShowing(false);   //set all to not be showing
      
      ofPushStyle();
      ofFill();
      ofSetColor(50,50,50,gModuleDrawAlpha*.5f);
      ofRect(kLayoutControlsX,kLayoutControlsY,235,140);
      ofPopStyle();
      
      if (!mFoundLayoutFile)
         gFont.DrawStringWrap("couldn't load layout file at "+mLastLoadedLayoutFile+", using the default layout instead", 15, 3, kLayoutControlsY + 160, 235);
      
      if (mHighlightedLayoutElement != -1)
      {
         UIControlConnection* connection = GetConnectionForControl(mLayoutControls[mHighlightedLayoutElement].mType, mLayoutControls[mHighlightedLayoutElement].mControl);
         if (connection)
            connection->DrawLayout();
      }
   }
   
   if (mMappingDisplayMode == kList)
   {
      ofPushStyle();
      if (mIsConnected)
      {
         ofSetColor(ofColor::green);
         DrawTextNormal("connected", 165, 13);
      }
      else
      {
         ofSetColor(ofColor::red);
         DrawTextNormal("not connected", 165, 13);
      }
      ofPopStyle();
      
      float w,h;
      GetDimensions(w, h);
      mAddConnectionButton->SetPosition(mAddConnectionButton->GetPosition(true).x, h-17);
      mAddConnectionButton->Draw();
      
      //DrawTextNormal("                                                                                                                                                                MIDI out                           all", 12, 34);
      //DrawTextNormal("midi       num   chan    path                                                           type       value        inc        feedback    off    on    blink  pages", 12, 46);
      
      std::list<UIControlConnection*> toDraw;
      //draw pageless ones first
      for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
      {
         UIControlConnection* connection = *iter;
         connection->SetShowing(false);   //set all to not be showing
         if (connection->mPageless)
         {
            toDraw.push_back(connection);
         }
      }
      //draw ones on this page second
      for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
      {
         UIControlConnection* connection = *iter;
         if (connection->mPage == mControllerPage && connection->mPageless == false)
         {
            toDraw.push_back(connection);
         }
      }
      
      int i=0;
      for (auto iter = toDraw.begin(); iter != toDraw.end();)
      {
         UIControlConnection* connection = *iter;
         ++iter;
         UIControlConnection* next = nullptr;
         if (iter != toDraw.end())
            next = *iter;
         connection->SetNext(next);
         connection->DrawList(i);

         ControlLayoutElement& control = GetLayoutControl(connection->mControl, connection->mMessageType);
         if (control.mControlCable)
         {
            int x = 356;
            int y = 59 + 20 * i;

            control.mControlCable->SetEnabled(UIControlConnection::sDrawCables);
            control.mControlCable->SetManualPosition(x, y);
            control.mControlCable->SetPatchCableDrawMode(kPatchCableDrawMode_Normal);

            control.mControlCable->SetTarget(connection->mUIControl);
            if (connection->mUIControl)
               control.mControlCable->GetPatchCables()[0]->SetUIControlConnection(connection);
         }
         ++i;
      }
   }
}

namespace
{
   const int kHoveredLayoutElement_GridOffset = 1000;
}

void MidiController::DrawModuleUnclipped()
{
   const float kDisplayMs = 500;
   std::string displayString;
   
   IUIControl* drawControl = nullptr;
   if (gTime < mLastBoundControlTime + kDisplayMs)
   {
      drawControl = mLastBoundUIControl;
      if (drawControl != nullptr)
         displayString = drawControl->Path() + " bound!";
   }
   else if (gTime < mLastConnectedActivityTime + kDisplayMs)
   {
      drawControl = mLastActivityUIControl;
      if (drawControl != nullptr)
         displayString = drawControl->Path() + ": " + drawControl->GetDisplayValue(drawControl->GetValue());
   }

   if (!displayString.empty() && drawControl != nullptr)
   {
      ofPushMatrix();
      ofTranslate(-TheSynth->GetDrawOffset().x, -TheSynth->GetDrawOffset().y);
      ofTranslate(-GetPosition().x, -GetPosition().y);
      ofPushStyle();
      ofFill();
      ofVec2f pos(50, TheSynth->GetDrawRect().height - 100);
      const float kWidth = 500;
      const float kHeight = 70;
      ofSetColor(80, 80, 80);
      ofRect(pos.x, pos.y, kWidth, kHeight);
      ofSetColor(120, 120, 120);
      ofRect(pos.x, pos.y, kWidth * drawControl->GetMidiValue(), kHeight);
      ofSetColor(255, 255, 255);
      DrawTextBold(displayString, pos.x + 20, pos.y + 50, 40);
      ofPopStyle();
      ofPopMatrix();
   }

   if (mHoveredLayoutElement != -1)
   {
      std::string tooltip;
      ofVec2f pos;
      if (mHoveredLayoutElement < kHoveredLayoutElement_GridOffset)
      {
         tooltip = GetLayoutTooltip(mHoveredLayoutElement);
         pos = mLayoutControls[mHoveredLayoutElement].mPosition;
         pos.x += mLayoutControls[mHoveredLayoutElement].mDimensions.x + 3;
         pos.y += mLayoutControls[mHoveredLayoutElement].mDimensions.y / 2;
      }
      else
      {
         tooltip = "grid";
         auto* grid = mGrids[mHoveredLayoutElement - kHoveredLayoutElement_GridOffset];
         pos = grid->mPosition;
         pos.x += 18;
      }

      float width = GetStringWidth(tooltip);

      ofFill();
      ofSetColor(50, 50, 50);
      ofRect(pos.x, pos.y, width+10, 15);

      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofSetColor(255, 255, 255);
      DrawTextNormal(tooltip, pos.x + 5, pos.y + 12);
   }
}

std::string MidiController::GetLayoutTooltip(int controlIndex)
{
   if (controlIndex >= 0 && controlIndex < mLayoutControls.size())
   {
      if (mNonstandardController != nullptr)
         return mNonstandardController->GetControlTooltip(mLayoutControls[controlIndex].mType, mLayoutControls[controlIndex].mControl);
      return GetDefaultTooltip(mLayoutControls[controlIndex].mType, mLayoutControls[controlIndex].mControl);
   }
   return "";
}

//static
std::string MidiController::GetDefaultTooltip(MidiMessageType type, int control)
{
   std::string str;
   if (type == kMidiMessage_Note)
      str = "note ";
   else if (type == kMidiMessage_Control)
      str = "cc ";
   else if (type == kMidiMessage_PitchBend)
      return "pitchbend";
   else if (type == kMidiMessage_Program)
      str = "program ";
   return str + ofToString(control);
}

UIControlConnection *MidiController::GetConnectionForCableSource(const PatchCableSource *source)
{
   for (auto c : mLayoutControls)
   {
      if (c.mActive && c.mControlCable == source)
      {
         return GetConnectionForControl(c.mType, c.mControl);
      }
   }
   return nullptr;
}

void MidiController::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   if (right)
      return;
   
   if (mMappingDisplayMode == kLayout)
   {
      bool selected = false;
      for (int i=0; i<NUM_LAYOUT_CONTROLS; ++i)
      {
         ControlLayoutElement& control = mLayoutControls[i];
         if (control.mActive)
         {
            ofRectangle controlRect(control.mPosition.x, control.mPosition.y, control.mDimensions.x, control.mDimensions.y);
            if (controlRect.contains(x, y))
            {
               mHighlightedLayoutElement = i;
               selected = true;
               break;
            }
         }
      }
      //if (!selected)
      //   mHighlightedLayoutElement = -1;
   }
}

bool MidiController::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mHoveredLayoutElement = -1;

   if (mMappingDisplayMode == kLayout)
   {
      for (int i = 0; i < NUM_LAYOUT_CONTROLS; ++i)
      {
         ControlLayoutElement& control = mLayoutControls[i];
         if (control.mActive)
         {
            ofRectangle controlRect(control.mPosition.x, control.mPosition.y, control.mDimensions.x, control.mDimensions.y);
            if (controlRect.contains(x, y))
            {
               mHoveredLayoutElement = i;
               control.mControlCable->SetPatchCableDrawMode(kPatchCableDrawMode_Normal);
            }
            else
            {
               control.mControlCable->SetPatchCableDrawMode(kPatchCableDrawMode_SourceOnHoverOnly);
            }
         }
      }
      
      int i = 0;
      for (auto* grid : mGrids)
      {
         ofRectangle controlRect(grid->mPosition.x - 5, grid->mPosition.y - 5, 10, 10);
         if (controlRect.contains(x, y))
            mHoveredLayoutElement = i + kHoveredLayoutElement_GridOffset;
         ++i;
      }
   }

   return false;
}

bool MidiController::IsInputConnected(bool immediate)
{
   if (mNonstandardController != nullptr)
      return mNonstandardController->IsInputConnected();
   return mDevice.IsInputConnected(immediate);
}

int MidiController::GetNumConnectionsOnPage(int page)
{
   int i=0;
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (connection->mPage == mControllerPage || connection->mPageless)
         ++i;
   }
   return i;
}

void MidiController::SetEntirePageToZero(int page)
{
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (connection->mPage == page && connection->mPageless == false)
      {
         if (connection->mMessageType == kMidiMessage_Control)
            mDevice.SendCC(connection->mControl, 0, connection->mChannel);
         if (connection->mMessageType == kMidiMessage_Note)
            mDevice.SendNote(gTime, connection->mControl, 0, false, connection->mChannel);
      }
   }
}

void MidiController::HighlightPageControls(int page)
{
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (connection->mPage == page && connection->mPageless == false)
      {
         if (connection->mUIControl)
            connection->mUIControl->StartBeacon();
      }
   }
}

void MidiController::GetModuleDimensions(float& width, float& height)
{
   if (mMappingDisplayMode == kList)
   {
      width = 880;
      height = 72 + 20 * GetNumConnectionsOnPage(mControllerPage);
   }
   else if (mMappingDisplayMode == kLayout)
   {
      width = MAX(408, mLayoutWidth);
      height = MAX(145, mLayoutHeight);
   }
   else
   {
      width = 163;
      height = 54;
   }
}

void MidiController::ResyncTwoWay()
{
   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
   {
      (*i)->mLastControlValue = -1;
   }
}

void MidiController::SendNote(int page, int pitch, int velocity, bool forceNoteOn /*= false*/, int channel /*= -1*/)
{
   if (channel == -1)
      channel = mOutChannel;

   if (page == mControllerPage)
   {
      mDevice.SendNote(gTime, pitch,velocity,forceNoteOn, channel);
      
      if (mNonstandardController)
         mNonstandardController->SendValue(page, pitch, velocity/127.0f, forceNoteOn, channel);
   }
}

void MidiController::SendCC(int page, int ctl, int value, int channel /*= -1*/)
{
   if (channel == -1)
      channel = mOutChannel;

   if (page == mControllerPage)
   {
      mDevice.SendCC(ctl, value, channel);

      if (mNonstandardController)
         mNonstandardController->SendValue(page, ctl, value / 127.0f, channel);
   }
}

void MidiController::SendProgramChange(int page, int program, int channel /*= -1*/)
{
   if (channel == -1)
      channel = mOutChannel;

   if (page == mControllerPage)
   {
      mDevice.SendProgramChange(program, channel);
   }
}

void MidiController::SendPitchBend(int page, int bend, int channel /*= -1*/)
{
   if (channel == -1)
      channel = mOutChannel;

   if (page == mControllerPage)
      mDevice.SendPitchBend(bend, channel);
}

void MidiController::SendData(int page, unsigned char a, unsigned char b, unsigned char c)
{
   if (page == mControllerPage)
   {
      mDevice.SendData(a,b,c);
   }
}

UIControlConnection* MidiController::GetConnectionForControl(MidiMessageType messageType, int control)
{
   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
   {
      UIControlConnection* connection = *i;
      if (connection->mMessageType == messageType &&
          connection->mControl == control &&
          (connection->mPageless || connection->mPage == mControllerPage))
         return connection;
   }
   return nullptr;
}

ControlLayoutElement& MidiController::GetLayoutControl(int control, MidiMessageType type)
{
   int index = NUM_LAYOUT_CONTROLS - 1;
   if (type == kMidiMessage_Control)
      index = control;
   else if (type == kMidiMessage_Note)
      index = control + 128;
   else if (type == kMidiMessage_Program)
      index = control + 128 + 128;
   else if (type == kMidiMessage_PitchBend)
      index = 128 + 128 + 128;
   return mLayoutControls[index];
}

void MidiController::LoadLayout(std::string filename)
{
   mLastLoadedLayoutFile = ofToDataPath("controllers/"+filename);
   for (int i = 0; i < mLayoutFileDropdown->GetNumValues(); ++i)
   {
      if (filename == mLayoutFileDropdown->GetLabel(i))
         mLayoutFileIndex = i;
   }
   
   for (int i = 0; i < NUM_LAYOUT_CONTROLS; ++i)
   {
      mLayoutControls[i].mActive = false;
      RemovePatchCableSource(mLayoutControls[i].mControlCable);
      mLayoutControls[i].mControlCable = nullptr;
   }
   for (auto grid : mGrids)
   {
      RemovePatchCableSource(grid->mGridCable);
      delete grid;
   }
   mGrids.clear();
   
   bool useDefaultLayout = true;
   bool loaded = mLayoutData.open(mLastLoadedLayoutFile);
   try
   {
      if (loaded)
      {
         if (mNonstandardController != nullptr)
            mNonstandardController->SetLayoutData(mLayoutData);

         mFoundLayoutFile = true;
         if (!mLayoutData["outchannel"].isNull())
         {
            mOutChannel = mLayoutData["outchannel"].asInt();
            mModuleSaveData.SetInt("outchannel", mOutChannel);
         }
         if (!mLayoutData["outdevice"].isNull())
         {
            mDeviceOut = mLayoutData["outdevice"].asString();
            mModuleSaveData.SetString("deviceout", mDeviceOut);
            mTwoWay = true;
            mDevice.ConnectOutput(mDeviceOut.c_str(), mOutChannel);
         }
         if (!mLayoutData["usechannelasvoice"].isNull())
         {
            SetUseChannelAsVoice(mLayoutData["usechannelasvoice"].asBool());
            mModuleSaveData.SetBool("usechannelasvoice", mUseChannelAsVoice);
         }
         if (!mLayoutData["pitchbendrange"].isNull())
         {
            SetPitchBendRange(mLayoutData["pitchbendrange"].asInt());
            mModuleSaveData.SetFloat("pitchbendrange", mPitchBendRange);
         }
         if (!mLayoutData["modwheelcc"].isNull())
         {
            mModwheelCC = mLayoutData["modwheelcc"].asInt();
            mModuleSaveData.SetInt("modwheelcc(1or74)", mModwheelCC);
         }
         if (!mLayoutData["modwheeloffset"].isNull())
         {
            mModWheelOffset = mLayoutData["modwheeloffset"].asDouble();
            mModuleSaveData.SetFloat("modwheeloffset", mModWheelOffset);
         }
         if (!mLayoutData["pressureoffset"].isNull())
         {
            mPressureOffset = mLayoutData["pressureoffset"].asDouble();
            mModuleSaveData.SetFloat("pressureoffset", mPressureOffset);
         }
         if (!mLayoutData["twoway_on_change"].isNull())
         {
            mSendTwoWayOnChange = mLayoutData["twoway_on_change"].asBool();
            mModuleSaveData.SetBool("twoway_on_change", mSendTwoWayOnChange);
         }
         if (!mLayoutData["resend_feedback_on_release"].isNull())
         {
            mResendFeedbackOnRelease = mLayoutData["resend_feedback_on_release"].asBool();
            mModuleSaveData.SetBool("resend_feedback_on_release", mResendFeedbackOnRelease);
         }
         if (!mLayoutData["channelfilter"].isNull())
         {
            const int layoutChannelFilter = mLayoutData["channelfilter"].asInt();
            if (layoutChannelFilter >= (int)ChannelFilter::k1 && layoutChannelFilter <= (int)ChannelFilter::k16)
            {
               mChannelFilter = (ChannelFilter)layoutChannelFilter;
               // The enum is restored by casting from an int
               mModuleSaveData.SetInt("channelfilter", layoutChannelFilter);
            }
         }
         if (!mLayoutData["groups"].isNull())
         {
            useDefaultLayout = false;
            for (int group = 0; group < mLayoutData["groups"].size(); ++group)
            {
               int rows = mLayoutData["groups"][group]["rows"].asInt();
               int cols = mLayoutData["groups"][group]["cols"].asInt();
               ofVec2f pos;
               pos.x = (mLayoutData["groups"][group]["position"])[0u].asDouble();
               pos.y = (mLayoutData["groups"][group]["position"])[1u].asDouble();
               ofVec2f dim;
               dim.x = (mLayoutData["groups"][group]["dimensions"])[0u].asDouble();
               dim.y = (mLayoutData["groups"][group]["dimensions"])[1u].asDouble();
               ofVec2f spacing;
               spacing.x = (mLayoutData["groups"][group]["spacing"])[0u].asDouble();
               spacing.y = (mLayoutData["groups"][group]["spacing"])[1u].asDouble();
               MidiMessageType messageType;
               if (mLayoutData["groups"][group]["messageType"] == "control")
                  messageType = kMidiMessage_Control;
               if (mLayoutData["groups"][group]["messageType"] == "note")
                  messageType = kMidiMessage_Note;
               if (mLayoutData["groups"][group]["messageType"] == "pitchbend")
                  messageType = kMidiMessage_PitchBend;
               if (mLayoutData["groups"][group]["messageType"] == "program")
                  messageType = kMidiMessage_Program;
               ControlDrawType drawType;
               if (mLayoutData["groups"][group]["drawType"] == "button")
                  drawType = kDrawType_Button;
               if (mLayoutData["groups"][group]["drawType"] == "knob")
                  drawType = kDrawType_Knob;
               if (mLayoutData["groups"][group]["drawType"] == "slider")
                  drawType = kDrawType_Slider;
               bool incremental = false;
               if (!mLayoutData["groups"][group]["incremental"].isNull())
                  incremental = mLayoutData["groups"][group]["incremental"].asBool();
               int offVal = 0;
               int onVal = 127;
               if (!mLayoutData["groups"][group]["colors"].isNull() &&
                  mLayoutData["groups"][group]["colors"].size() > 2)
               {
                  offVal = mLayoutData["groups"][group]["colors"][0u].asInt();
                  onVal = mLayoutData["groups"][group]["colors"][2u].asInt();
               }
               ControlType connectionType = kControlType_Default;
               if (mLayoutData["groups"][group]["connection_type"] == "slider")
                  connectionType = kControlType_Slider;
               if (mLayoutData["groups"][group]["connection_type"] == "set")
                  connectionType = kControlType_SetValue;
               if (mLayoutData["groups"][group]["connection_type"] == "release")
                  connectionType = kControlType_SetValueOnRelease;
               if (mLayoutData["groups"][group]["connection_type"] == "toggle")
                  connectionType = kControlType_Toggle;
               if (mLayoutData["groups"][group]["connection_type"] == "direct")
                  connectionType = kControlType_Direct;
               for (int row = 0; row < rows; ++row)
               {
                  for (int col = 0; col < cols; ++col)
                  {
                     int index = col + row * cols;
                     int control = mLayoutData["groups"][group]["controls"][index].asInt();
                     GetLayoutControl(control, messageType).Setup(this, messageType, control, drawType, incremental, offVal, onVal, false, connectionType, pos.x + kLayoutButtonsX + spacing.x*col, pos.y + kLayoutButtonsY + spacing.y*row, dim.x, dim.y);

                     //clear out values on controllers
                     /*if (messageType == kMidiMessage_Note)
                        SendNote(0, control, offVal, true);
                     if (messageType == kMidiMessage_Control)
                        SendCC(0, control, offVal);*/
                  }
               }

               if (drawType == kDrawType_Button && rows * cols >= 8) //we're a button grid
               {
                  GridLayout* grid = new GridLayout();
                  grid->mRows = rows;
                  grid->mCols = cols;
                  grid->mPosition.set(pos.x + kLayoutButtonsX - 2, pos.y + kLayoutButtonsY - 2);
                  grid->mDimensions.set(spacing.x*cols + 2, spacing.y*rows + 2);
                  grid->mType = messageType;
                  grid->mGridCable = new PatchCableSource(this, kConnectionType_Grid);
                  grid->mGridCable->SetManualPosition(pos.x + kLayoutButtonsX - 2, pos.y + kLayoutButtonsY - 2);
                  grid->mGridCable->AddTypeFilter("gridcontroller");
                  AddPatchCableSource(grid->mGridCable);

                  for (int row = 0; row < rows; ++row)
                  {
                     for (int col = 0; col < cols; ++col)
                     {
                        int index = col + row * cols;
                        int control = mLayoutData["groups"][group]["controls"][index].asInt();
                        grid->mControls.push_back(control);
                     }
                  }

                  if (!mLayoutData["groups"][group]["colors"].isNull() &&
                     mLayoutData["groups"][group]["colors"].size() > 0)
                  {
                     for (int i = 0; i < mLayoutData["groups"][group]["colors"].size(); ++i)
                        grid->mColors.push_back(mLayoutData["groups"][group]["colors"][i].asInt());
                  }
                  else
                  {
                     grid->mColors.push_back(0);
                     grid->mColors.push_back(127);
                  }

                  mGrids.push_back(grid);
               }
            }
         }
      }
   }
   catch (Json::LogicError& e)
   {
      loaded = false;
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }
   
   if (!loaded)
   {
      mFoundLayoutFile = false;
      mLayoutData.clear();
   }
   
   if (useDefaultLayout)
   {
      const float kSpacingX = 20;
      const float kSpacingY = 20;

      for (int i=0; i<128; ++i)
      {
         GetLayoutControl(i, kMidiMessage_Control).
            Setup(this, kMidiMessage_Control, i, kDrawType_Slider, false, 0, 127, true, kControlType_Default, i%8 * kSpacingX + kLayoutButtonsX + 9, i/8 * kSpacingY + kLayoutButtonsY, kSpacingX * .666f, kSpacingY * .93f);
         GetLayoutControl(i, kMidiMessage_Note).
            Setup(this, kMidiMessage_Note, i, kDrawType_Button, false, 0, 127, true, kControlType_Default, i%8 * kSpacingX + 8 * kSpacingX + kLayoutButtonsX + 15, i/8 * kSpacingY + kLayoutButtonsY, kSpacingX * .93f, kSpacingY * .93f);
      }
      
      GetLayoutControl(0, kMidiMessage_PitchBend).
         Setup(this, kMidiMessage_PitchBend, 0, kDrawType_Slider, false, 0, 127, true, kControlType_Default, kLayoutButtonsX + kSpacingX * 17, kLayoutButtonsY, 25, 100);
   }
   
   mLayoutWidth = 0;
   mLayoutHeight = 300;
   for (int i=0; i<NUM_LAYOUT_CONTROLS; ++i)
   {
      if (mLayoutControls[i].mActive)
      {
         mLayoutWidth = MAX(mLayoutWidth, mLayoutControls[i].mPosition.x + mLayoutControls[i].mDimensions.x + 5);
         mLayoutHeight = MAX(mLayoutHeight, mLayoutControls[i].mPosition.y + mLayoutControls[i].mDimensions.y + 20);
      }
   }
}

void MidiController::OnDeviceChanged()
{
   std::string filename = mDeviceIn + ".json";
   ofStringReplace(filename, "/", "");
   LoadLayout(filename);

   mModulation.GetModWheel(-1)->SetValue(mModWheelOffset);
   mModulation.GetPressure(-1)->SetValue(mPressureOffset);
}

void MidiController::CheckboxUpdated(Checkbox* checkbox)
{
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (checkbox == connection->mPagelessCheckbox)
      {
         if (connection->mPageless == false) //just made this not pageless
            connection->mPage = mControllerPage;   //make the current page this connection's page
      }
   }
}

void MidiController::ButtonClicked(ClickButton* button)
{
   if (button == mAddConnectionButton)
   {
      AddControlConnection(kMidiMessage_Control, 0, -1, nullptr);
   }
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (button == connection->mRemoveButton)
      {
         mConnections.remove(connection);
         delete connection;
         break;
      }
      if (button == connection->mCopyButton)
      {
         UIControlConnection* copy = connection->MakeCopy();
         copy->CreateUIControls((int)mConnections.size());
         mConnections.push_back(copy); //make a copy of this one
         break;
      }
   }
}

void MidiController::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mPageSelector)
   {
      SetEntirePageToZero(oldVal);
      if (mControllerPage >= 0 && mControllerPage < mListeners.size())
      {
         for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
            (*i)->ControllerPageSelected();
      }
      for (int i=0; i<NUM_LAYOUT_CONTROLS; ++i)
      {
         if (mLayoutControls[i].mActive)
         {
            UIControlConnection* connection = GetConnectionForControl(mLayoutControls[i].mType, mLayoutControls[i].mControl);
            if (connection && mLayoutControls[i].mControlCable)
               mLayoutControls[i].mControlCable->SetTarget(connection->mUIControl);
         }
      }
      for (auto* grid : mGrids)
      {
         if (grid->mGridControlTarget[mControllerPage] != nullptr)
         {
            grid->mGridCable->SetTarget(grid->mGridControlTarget[mControllerPage]);
         }
         else
         {
            grid->mGridCable->ClearPatchCables();
         }
      }
      HighlightPageControls(mControllerPage);
      ResyncTwoWay();
   }
   if (list == mControllerList)
   {
      if (TheSynth->GetTopModalFocusItem() == mControllerList->GetModalDropdown())
      {
         ConnectDevice();
         OnDeviceChanged();
      }
      else
      {
         UpdateControllerIndex();
      }
   }
   if (list == mMonomeDeviceDropdown)
   {
      Monome* monome = dynamic_cast<Monome*>(mNonstandardController);
      if (monome)
         monome->ConnectToDevice(mMonomeDeviceDropdown->GetLabel(mMonomeDeviceIndex));
   }
   if (list == mLayoutFileDropdown)
   {
      LoadLayout(mLayoutFileDropdown->GetLabel(mLayoutFileIndex));
   }
}

void MidiController::DropdownClicked(DropdownList* list)
{
   if (list == mControllerList)
   {
      BuildControllerList();
   }
}

void MidiController::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   if (radio == mMappingDisplayModeSelector)
   {
      mAddConnectionButton->SetShowing(mMappingDisplayMode == kList);
   }
}

void MidiController::TextEntryActivated(TextEntry* entry)
{
   //if you click a text entry while a UI control is in bind mode, use that one's path
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (entry == connection->mUIControlPathEntry)
      {
         if (gBindToUIControl)
         {
            if (connection->mUIControl)
               connection->mUIControl->RemoveRemoteController();
            connection->mUIControl = gBindToUIControl;
            gBindToUIControl->AddRemoteController();
            gBindToUIControl = nullptr;
            IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));
         }
      }
   }
}

void MidiController::TextEntryComplete(TextEntry* entry)
{
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      UIControlConnection* connection = *iter;
      if (entry == connection->mUIControlPathEntry)
         connection->SetUIControl(connection->mUIControlPathInput);
   }

   if (entry == mOscInPortEntry)
   {
      OscController* osc = dynamic_cast<OscController*>(mNonstandardController);
      if (osc != nullptr)
         osc->SetInPort(mOscInPort);
   }
}

void MidiController::PreRepatch(PatchCableSource* cableSource)
{
   for (auto* grid : mGrids)
   {
      if (cableSource == grid->mGridCable)
      {
         grid->mGridControlTarget[mControllerPage] = dynamic_cast<GridControlTarget*>(cableSource->GetTarget());
         if (grid->mGridControlTarget[mControllerPage] && grid->mGridControlTarget[mControllerPage]->GetGridController())
            dynamic_cast<GridControllerMidi*>(grid->mGridControlTarget[mControllerPage]->GetGridController())->UnhookController();
         return;
      }
   }
}

void MidiController::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (auto* grid : mGrids)
   {
      if (cableSource == grid->mGridCable)
      {
         grid->mGridControlTarget[mControllerPage] = dynamic_cast<GridControlTarget*>(cableSource->GetTarget());
         if (grid->mGridControlTarget[mControllerPage])
         {
            GridControllerMidi* gridController = new GridControllerMidi();
            grid->mGridControlTarget[mControllerPage]->SetGridController(gridController);
            gridController->SetUp(grid, mControllerPage, this);
         }
         return;
      }
   }
   
   bool repatched = false;
   for (auto* connection : mConnections)
   {
      repatched = connection->PostRepatch(cableSource, fromUserClick);
      if (repatched)
         break;
   }
   
   if (cableSource->GetTarget())   //need to make connection
   {
      if (fromUserClick && !repatched)
      {
         int layoutControl = GetLayoutControlIndexForCable(cableSource);
         if (layoutControl != -1)
         {
            UIControlConnection* connection = AddControlConnection(mLayoutControls[layoutControl].mType, mLayoutControls[layoutControl].mControl, -1, dynamic_cast<IUIControl*>(cableSource->GetTarget()));

            RadioButton* radioButton = dynamic_cast<RadioButton*>(cableSource->GetTarget());
            if (radioButton && mLayoutControls[layoutControl].mDrawType == kDrawType_Button)
            {
               connection->mType = kControlType_SetValue;
               float closestSq = FLT_MAX;
               int closestIdx = 0;
               ofVec2f mousePos(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
               for (int i=0; i<radioButton->GetNumValues(); ++i)
               {
                  float distSq = (mousePos - radioButton->GetOptionPosition(i)).distanceSquared();
                  if (distSq < closestSq)
                  {
                     closestSq = distSq;
                     closestIdx = i;
                  }
               }
               connection->mValue = closestIdx;
            }
         }
      }
   }
   else // need to remove a connection if no cables remain
   {
      auto uiConnection = GetConnectionForCableSource(cableSource);
      mConnections.remove(uiConnection);
      delete uiConnection;
   }
}

void MidiController::OnCableGrabbed(PatchCableSource* cableSource)
{
   int layoutControl = GetLayoutControlIndexForCable(cableSource);
   if (layoutControl != -1)
   {
      mHighlightedLayoutElement = layoutControl;
   }
}

int MidiController::GetLayoutControlIndexForCable(PatchCableSource* cable) const
{
   for (int i=0; i<NUM_LAYOUT_CONTROLS; ++i)
   {
      if (mLayoutControls[i].mControlCable == cable)
         return i;
   }
   return -1;
}

int MidiController::GetLayoutControlIndexForMidi(MidiMessageType type, int control) const
{
   for (int i = 0; i < NUM_LAYOUT_CONTROLS; ++i)
   {
      if (mLayoutControls[i].mType == type && mLayoutControls[i].mControl == control)
         return i;
   }
   return -1;
}

static std::vector<std::string> sCachedInputDevices;
//static
std::vector<std::string> MidiController::GetAvailableInputDevices()
{
   if (!juce::MessageManager::existsAndIsCurrentThread())
      return sCachedInputDevices;
   
   std::vector<std::string> devices;
   for (auto& d : MidiInput::getAvailableDevices())
   {
      if (d.identifier == "blah")   //my BCF-2000 and BCR-2000 both report as a BCF-2000, come up with some hack here to name it correctly
         devices.push_back("BCR-2000");
      else
         devices.push_back(d.name.toStdString());
   }

   devices.push_back("monome");
   devices.push_back("osccontroller");
   
   sCachedInputDevices = devices;

   return devices;
}

static std::vector<std::string> sCachedOutputDevices;
//static
std::vector<std::string> MidiController::GetAvailableOutputDevices()
{
   if (!juce::MessageManager::existsAndIsCurrentThread())
      return sCachedOutputDevices;
   
   std::vector<std::string> devices;
   for (auto& d : MidiOutput::getDevices())
      devices.push_back(d.toStdString());

   devices.push_back("monome");
   devices.push_back("osccontroller");
   
   sCachedOutputDevices = devices;

   return devices;
}

namespace {
   void FillMidiInput(DropdownList* list)
   {
      assert(list);
      const auto& devices = MidiController::GetAvailableInputDevices();
      for (int i=0; i< devices.size(); ++i)
         list->AddLabel(devices[i].c_str(), i);
   }

   void FillMidiOutput(DropdownList* list)
   {
      assert(list);
      const auto& devices = MidiController::GetAvailableOutputDevices();
      for (int i = 0; i < devices.size(); ++i)
         list->AddLabel(devices[i].c_str(), i);
   }
}

void MidiController::BuildControllerList()
{
   mControllerList->Clear();
   FillMidiInput(mControllerList);
}

void MidiController::ConnectDevice()
{
   mDevice.DisconnectInput();
   mDevice.DisconnectOutput();
   ResyncTwoWay();

   std::string deviceInName = mControllerList->GetLabel(mControllerIndex);
   std::string deviceOutName = String(deviceInName).replace("Input", "Output").replace("input", "output").toStdString();
   bool hasOutput = MidiOutput::getDevices().contains(String(deviceOutName));
   mDeviceIn = deviceInName;
   mDeviceOut = hasOutput ? deviceOutName : "";
   mModuleSaveData.SetString("devicein", mDeviceIn);
   mModuleSaveData.SetString("deviceout", mDeviceOut);

   if (mDeviceIn == "xboxcontroller")
   {
      //TODO_PORT(Ryan)
      
      //Xbox360Controller* xbox = new Xbox360Controller(this);
      //mNonstandardController = xbox;
   }
   else if (mDeviceIn == "monome")
   {
      if (dynamic_cast<Monome*>(mNonstandardController) == nullptr)
      {
         Monome* monome = new Monome(this);
         monome->UpdateDeviceList(mMonomeDeviceDropdown);
         mNonstandardController = monome;
      }
   }
   else if (mDeviceIn == "osccontroller")
   {
      if (dynamic_cast<OscController*>(mNonstandardController) == nullptr)
      {
         OscController* osc = new OscController(this,"",-1,mOscInPort);
         mNonstandardController = osc;
      }
   }
   else if (mDeviceIn == "midicapturer")
   {
      if (dynamic_cast<MidiCapturerDummyController*>(mNonstandardController) == nullptr)
      {
         MidiCapturerDummyController* cap = new MidiCapturerDummyController(this);
         mNonstandardController = cap;
      }
   }
   else if (mDeviceIn.length() > 0) //not one of the special options, must be midi
   {
      if (mNonstandardController != nullptr)
      {
         delete mNonstandardController;
         mNonstandardController = nullptr;
      }

      mDevice.ConnectInput(mDeviceIn.c_str());
   }
   
   if (mDeviceOut.length() > 0 && mNonstandardController == nullptr)
   {
      mTwoWay = true;
      mDevice.ConnectOutput(mDeviceOut.c_str(), mOutChannel);
   }
   
   if (mNonstandardController != nullptr)
      mNonstandardController->SetLayoutData(mLayoutData);

   mIsConnected = IsInputConnected(K(immediate));
   
   mInitialConnectionTime = gTime;
}

void MidiController::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("devicein", moduleInfo, "", FillMidiInput);
   mModuleSaveData.LoadString("deviceout", moduleInfo, "", FillMidiOutput);
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("velocitymult",moduleInfo,1,0,10,K(isTextField));
   mModuleSaveData.LoadBool("usechannelasvoice",moduleInfo,false);
   EnumMap channelMap;
   channelMap["any"] = (int)ChannelFilter::kAny;
   channelMap["01"] = (int)ChannelFilter::k1;
   channelMap["02"] = (int)ChannelFilter::k2;
   channelMap["03"] = (int)ChannelFilter::k3;
   channelMap["04"] = (int)ChannelFilter::k4;
   channelMap["05"] = (int)ChannelFilter::k5;
   channelMap["06"] = (int)ChannelFilter::k6;
   channelMap["07"] = (int)ChannelFilter::k7;
   channelMap["08"] = (int)ChannelFilter::k8;
   channelMap["09"] = (int)ChannelFilter::k9;
   channelMap["10"] = (int)ChannelFilter::k10;
   channelMap["11"] = (int)ChannelFilter::k11;
   channelMap["12"] = (int)ChannelFilter::k12;
   channelMap["13"] = (int)ChannelFilter::k13;
   channelMap["14"] = (int)ChannelFilter::k14;
   channelMap["15"] = (int)ChannelFilter::k15;
   channelMap["16"] = (int)ChannelFilter::k16;
   mModuleSaveData.LoadEnum<ChannelFilter>("channelfilter", moduleInfo, (int)ChannelFilter::kAny, nullptr, &channelMap);
   mModuleSaveData.LoadInt("noteoffset",moduleInfo,0,-999,999,K(isTextField));
   mModuleSaveData.LoadFloat("pitchbendrange",moduleInfo,2,1,96,K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)",moduleInfo,1,0,127,K(isTextField));
   mModuleSaveData.LoadFloat("modwheeloffset", moduleInfo, 0, 0, 1, K(isTextField));
   mModuleSaveData.LoadFloat("pressureoffset", moduleInfo, 0, 0, 1, K(isTextField));
   
   mModuleSaveData.LoadInt("outchannel", moduleInfo, 1, 1, 16);
   
   mModuleSaveData.LoadBool("negativeedge",moduleInfo,false);
   mModuleSaveData.LoadBool("incrementalsliders", moduleInfo, false);
   mModuleSaveData.LoadBool("twoway_on_change", moduleInfo, true);
   mModuleSaveData.LoadBool("resend_feedback_on_release", moduleInfo, false);
   
   mConnectionsJson = moduleInfo["connections"];

   SetUpFromSaveData();
}

void MidiController::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetVelocityMult(mModuleSaveData.GetFloat("velocitymult"));
   SetUseChannelAsVoice(mModuleSaveData.GetBool("usechannelasvoice"));
   mChannelFilter = mModuleSaveData.GetEnum<ChannelFilter>("channelfilter");
   SetNoteOffset(mModuleSaveData.GetInt("noteoffset"));
   SetPitchBendRange(mModuleSaveData.GetFloat("pitchbendrange"));
   
   mModwheelCC = mModuleSaveData.GetInt("modwheelcc(1or74)");
   mModWheelOffset = mModuleSaveData.GetFloat("modwheeloffset");
   mPressureOffset = mModuleSaveData.GetFloat("pressureoffset");
   mModulation.GetModWheel(-1)->SetValue(mModWheelOffset);
   mModulation.GetPressure(-1)->SetValue(mPressureOffset);
   
   mDeviceIn = mModuleSaveData.GetString("devicein");
   mDeviceOut = mModuleSaveData.GetString("deviceout");
   mOutChannel = mModuleSaveData.GetInt("outchannel");
   assert(mOutChannel > 0 && mOutChannel <= 16);
   
   UseNegativeEdge(mModuleSaveData.GetBool("negativeedge"));
   mSlidersDefaultToIncremental = mModuleSaveData.GetBool("incrementalsliders");
   mSendTwoWayOnChange = mModuleSaveData.GetBool("twoway_on_change");
   mResendFeedbackOnRelease = mModuleSaveData.GetBool("resend_feedback_on_release");
   
   BuildControllerList();
   
   UpdateControllerIndex();

   ConnectDevice();
   
   OnDeviceChanged();
}

void MidiController::UpdateControllerIndex()
{
   mControllerIndex = -1;
   const auto& devices = GetAvailableInputDevices();
   for (int i=0; i<devices.size(); ++i)
   {
      if (devices[i].c_str() == mDeviceIn)
         mControllerIndex = i;
   }
}

void MidiController::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   mConnectionsJson.clear();
   mConnectionsJson.resize((int)mConnections.size());
   int i = 0;
   for (auto iter = mConnections.begin(); iter != mConnections.end(); ++iter)
   {
      const UIControlConnection* connection = *iter;
      mConnectionsJson[i]["control"] = connection->mControl;
      if (connection->mSpecialBinding == kSpecialBinding_Hover)
      {
         mConnectionsJson[i]["uicontrol"] = "hover";
      }
      else if (connection->mSpecialBinding >= kSpecialBinding_HotBind0 &&
               connection->mSpecialBinding <= kSpecialBinding_HotBind9)
      {
         mConnectionsJson[i]["uicontrol"] = "hotbind" + ofToString(connection->mSpecialBinding - kSpecialBinding_HotBind0);
      }
      else if (connection->mUIControl)
      {
         mConnectionsJson[i]["uicontrol"] = connection->mUIControl->Path();
      }
         
      if (connection->mMessageType == kMidiMessage_Control)
         mConnectionsJson[i]["type"] = "control";
      if (connection->mMessageType == kMidiMessage_Note)
         mConnectionsJson[i]["type"] = "note";
      if (connection->mMessageType == kMidiMessage_Program)
         mConnectionsJson[i]["type"] = "program";
      if (connection->mMessageType == kMidiMessage_PitchBend)
         mConnectionsJson[i]["type"] = "pitchbend";
         
      if (connection->mChannel != -1)
         mConnectionsJson[i]["channel"] = connection->mChannel;
      if (!connection->mPageless)
         mConnectionsJson[i]["page"] = connection->mPage;
         
      if (connection->mType == kControlType_SetValue)
         mConnectionsJson[i]["value"] = connection->mValue;
      else if (connection->mType == kControlType_Toggle)
         mConnectionsJson[i]["toggle"] = true;
      else if (connection->mType == kControlType_Direct)
         mConnectionsJson[i]["direct"] = true;
      else if (connection->mType == kControlType_SetValueOnRelease)
      {
         mConnectionsJson[i]["value"] = connection->mValue;
         mConnectionsJson[i]["release"] = true;
      }
         
      if (connection->mMidiOffValue != 0)
         mConnectionsJson[i]["midi_off_value"] = connection->mMidiOffValue;
      if (connection->mMidiOnValue != 127)
         mConnectionsJson[i]["midi_on_value"] = connection->mMidiOnValue;
      if (connection->mBlink)
         mConnectionsJson[i]["blink"] = true;
      if (connection->mIncrementAmount != 0)
         mConnectionsJson[i]["increment_amount"] = connection->mIncrementAmount;
      if (connection->mTwoWay == false)
         mConnectionsJson[i]["twoway"] = false;
      if (connection->mFeedbackControl != -1)
         mConnectionsJson[i]["feedbackcontrol"] = connection->mFeedbackControl;

      ++i;
   }
   
   int gridIndex = 0;
   for (auto* grid : mGrids)
   {
      mConnectionsJson[i]["grid_index"] = gridIndex;
      mConnectionsJson[i]["grid_pages"].resize(MAX_MIDI_PAGES);
      for (int page=0; page<MAX_MIDI_PAGES; ++page)
      {
         auto* target = grid->mGridControlTarget[page];
         if (target != nullptr)
            mConnectionsJson[i]["grid_pages"][page] = target->Path();
         else
            mConnectionsJson[i]["grid_pages"][page] = "";
      }
      ++i;
      ++gridIndex;
   }
   
   moduleInfo["connections"] = mConnectionsJson;
}

namespace
{
   const int kSaveStateRev = 1;
}

void MidiController::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;

   bool hasNonstandardController = (mNonstandardController != nullptr);
   out << hasNonstandardController;
   if (hasNonstandardController)
      mNonstandardController->SaveState(out);
}

void MidiController::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return;  //this was saved before we added versioning, bail out

   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);

   bool hasNonstandardController;
   in >> hasNonstandardController;
   if (hasNonstandardController)
   {
      assert(mNonstandardController != nullptr);
      mNonstandardController->LoadState(in);
   }
}

void UIControlConnection::SetUIControl(std::string path)
{
   if (mUIControl)
      mUIControl->RemoveRemoteController();
      
   if (path == "hover")
   {
      mUIControl = nullptr;
      mSpecialBinding = kSpecialBinding_Hover;
   }
   else if (path.substr(0,7) == "hotbind")
   {
      mUIControl = nullptr;
      int index = path[7] - '0';
      mSpecialBinding = SpecialControlBinding(int(kSpecialBinding_HotBind0) + index);
   }
   else
   {
      mUIControl = nullptr;
      try
      {
         mUIControl = mUIOwner->GetOwningContainer()->FindUIControl(path);
      }
      catch (std::exception e)
      {
      }
      mSpecialBinding = kSpecialBinding_None;
      
      if (mUIControl)
         mUIControl->AddRemoteController();
   }
}

void UIControlConnection::CreateUIControls(int index)
{
   if (!mEditorControls.empty())
   {
      TheSynth->LogEvent("Error setting up UIControlConnection", kLogEventType_Error);
      return;
   }
   
   static int sControlID = 0;
   mMessageTypeDropdown = new DropdownList(mUIOwner,"messagetype",12,-1,((int*)&mMessageType), 50);
   mControlEntry = new TextEntry(mUIOwner,"control",-1,-1,3,&mControl,0,127);
   mControlEntry->PositionTo(mMessageTypeDropdown, kAnchor_Right);
   mChannelDropdown = new DropdownList(mUIOwner,"channel",mControlEntry,kAnchor_Right,&mChannel, 40);
   mUIControlPathEntry = new TextEntry(mUIOwner,"path",-1,-1,25,mUIControlPathInput);
   mUIControlPathEntry->PositionTo(mChannelDropdown, kAnchor_Right);
   mControlTypeDropdown = new DropdownList(mUIOwner,"controltype",mUIControlPathEntry,kAnchor_Right,((int*)&mType),55);
   mValueEntry = new TextEntry(mUIOwner,"value",-1,-1,5,&mValue,-FLT_MAX,FLT_MAX);
   mValueEntry->PositionTo(mControlTypeDropdown, kAnchor_Right);
   mIncrementalEntry = new TextEntry(mUIOwner,"increment",-1,-1,4,&mIncrementAmount,-FLT_MAX,FLT_MAX);
   mIncrementalEntry->PositionTo(mValueEntry, kAnchor_Right);
   mTwoWayCheckbox = new Checkbox(mUIOwner,"twoway",mIncrementalEntry,kAnchor_Right,&mTwoWay);
   mFeedbackDropdown = new DropdownList(mUIOwner,"feedback",mTwoWayCheckbox,kAnchor_Right,&mFeedbackControl, 40);
   mMidiOffEntry = new TextEntry(mUIOwner,"midi off",-1,-1,3,&mMidiOffValue,0,127);
   mMidiOffEntry->PositionTo(mFeedbackDropdown, kAnchor_Right);
   mMidiOnEntry = new TextEntry(mUIOwner,"midi on",-1,-1,3,&mMidiOnValue,0,127);
   mMidiOnEntry->PositionTo(mMidiOffEntry, kAnchor_Right);
   mScaleOutputCheckbox = new Checkbox(mUIOwner,"scale",mMidiOnEntry,kAnchor_Right,&mScaleOutput);
   mBlinkCheckbox = new Checkbox(mUIOwner,"blink",mScaleOutputCheckbox,kAnchor_Right,&mBlink);
   mPagelessCheckbox = new Checkbox(mUIOwner,"pageless",mBlinkCheckbox,kAnchor_Right,&mPageless);
   mRemoveButton = new ClickButton(mUIOwner," x ",mPagelessCheckbox,kAnchor_Right);
   mCopyButton = new ClickButton(mUIOwner,"copy",mRemoveButton,kAnchor_Right);
   ++sControlID;
   
   mEditorControls.push_back(mMessageTypeDropdown);
   mEditorControls.push_back(mControlEntry);
   mEditorControls.push_back(mChannelDropdown);
   mEditorControls.push_back(mUIControlPathEntry);
   mEditorControls.push_back(mControlTypeDropdown);
   mEditorControls.push_back(mValueEntry);
   mEditorControls.push_back(mMidiOffEntry);
   mEditorControls.push_back(mMidiOnEntry);
   mEditorControls.push_back(mScaleOutputCheckbox);
   mEditorControls.push_back(mBlinkCheckbox);
   mEditorControls.push_back(mIncrementalEntry);
   mEditorControls.push_back(mTwoWayCheckbox);
   mEditorControls.push_back(mFeedbackDropdown);
   mEditorControls.push_back(mPagelessCheckbox);
   mEditorControls.push_back(mRemoveButton);
   mEditorControls.push_back(mCopyButton);
   
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
   {
      //(*iter)->SetNoHover(true);
      (*iter)->SetShouldSaveState(false);
   }
   
   mMessageTypeDropdown->AddLabel("note", kMidiMessage_Note);
   mMessageTypeDropdown->AddLabel("cc", kMidiMessage_Control);
   mMessageTypeDropdown->AddLabel("prgm", kMidiMessage_Program);
   mMessageTypeDropdown->AddLabel("bend", kMidiMessage_PitchBend);
   
   mChannelDropdown->AddLabel("any", -1);
   mChannelDropdown->AddLabel("0", 0);
   mChannelDropdown->AddLabel("1", 1);
   mChannelDropdown->AddLabel("2", 2);
   mChannelDropdown->AddLabel("3", 3);
   mChannelDropdown->AddLabel("4", 4);
   mChannelDropdown->AddLabel("5", 5);
   mChannelDropdown->AddLabel("6", 6);
   mChannelDropdown->AddLabel("7", 7);
   mChannelDropdown->AddLabel("8", 8);
   mChannelDropdown->AddLabel("9", 9);
   mChannelDropdown->AddLabel("10", 10);
   mChannelDropdown->AddLabel("11", 11);
   mChannelDropdown->AddLabel("12", 12);
   mChannelDropdown->AddLabel("13", 13);
   mChannelDropdown->AddLabel("14", 14);
   mChannelDropdown->AddLabel("15", 15);
   
   mControlTypeDropdown->AddLabel("slider", kControlType_Slider);
   mControlTypeDropdown->AddLabel("set", kControlType_SetValue);
   mControlTypeDropdown->AddLabel("release", kControlType_SetValueOnRelease);
   mControlTypeDropdown->AddLabel("toggle", kControlType_Toggle);
   mControlTypeDropdown->AddLabel("direct", kControlType_Direct);
   
   mFeedbackDropdown->AddLabel("self", -1);
   mFeedbackDropdown->AddLabel("none", -2);
   for (int i=0; i<=127; ++i)   //CCs
      mFeedbackDropdown->AddLabel(("cc"+ofToString(i)).c_str(), i);
   for (int i = 0; i <= 127; ++i)   //notes
      mFeedbackDropdown->AddLabel(("n" + ofToString(i)).c_str(), i+128);
}

void UIControlConnection::SetShowing(bool enabled)
{
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
      (*iter)->SetShowing(enabled);
}

void UIControlConnection::PreDraw()
{
   SetShowing(true);
   
   if (mUIControl || mSpecialBinding != kSpecialBinding_None)
   {
      if (mSpecialBinding == kSpecialBinding_Hover)
      {
         StringCopy(mUIControlPathInput, "hover", MAX_TEXTENTRY_LENGTH);
      }
      else if (mSpecialBinding >= kSpecialBinding_HotBind0 &&
               mSpecialBinding <= kSpecialBinding_HotBind9)
      {
         StringCopy(mUIControlPathInput, ("hotbind" + ofToString(mSpecialBinding - kSpecialBinding_HotBind0)).c_str(), MAX_TEXTENTRY_LENGTH);
      }
      else if (mUIControl != nullptr)
      {
         StringCopy(mUIControlPathInput, mUIControl->Path().c_str(), MAX_TEXTENTRY_LENGTH);
         if (mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable)
            mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable->SetTarget(mUIControl);
      }
      mUIControlPathEntry->SetInErrorMode(false);
   }
   else
   {
      mUIControlPathEntry->SetInErrorMode(true);
      if (PatchCable::sActivePatchCable == nullptr)
      {
         if (mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable)
            mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable->ClearPatchCables();
      }
   }
   
   if (mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable)
      mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable->SetEnabled(sDrawCables);
   
   mControlEntry->SetShowing(mMessageType != kMidiMessage_PitchBend);
   mValueEntry->SetShowing((mType == kControlType_SetValue|| mType == kControlType_SetValueOnRelease) && mIncrementAmount == 0);
   mIncrementalEntry->SetShowing(mType == kControlType_Slider || mType == kControlType_SetValue || mType == kControlType_SetValueOnRelease);
}

void UIControlConnection::DrawList(int index)
{
   PreDraw();
   
   mControlEntry->DrawLabel(false);
   mChannelDropdown->DrawLabel(false);
   mValueEntry->DrawLabel(false);
   mMidiOffEntry->DrawLabel(false);
   mMidiOnEntry->DrawLabel(false);
   mIncrementalEntry->DrawLabel(false);
   mFeedbackDropdown->DrawLabel(false);
   
   int y = 52 + 20 * index;
   
   IUIControl* lastControl = nullptr;
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
   {
      if (lastControl)
         (*iter)->PositionTo(lastControl, kAnchor_Right);
      else
         (*iter)->SetPosition(12,y);
      (*iter)->Draw();
      lastControl = *iter;
   }
   
   ofRectangle rect = mUIControlPathEntry->GetRect(true);
   if (mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable)
      mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable->SetManualPosition(rect.x + rect.width - 5, rect.y + rect.height/2);
   
   if (gTime - mLastActivityTime > 0 && gTime - mLastActivityTime < 200)
   {
      ofPushStyle();
      if (GetUIControl())
         ofSetColor(0,255,0,255*(1-(gTime - mLastActivityTime)/200));
      else
         ofSetColor(255,0,0,255*(1-(gTime - mLastActivityTime)/200));
      ofFill();
      ofRect(1,y+3,10,10);
      ofPopStyle();
   }
   
   /*if (mUIControlPathEntry == IKeyboardFocusListener::GetActiveKeyboardFocus() || TheSynth->InMidiMapMode())
   {
      IUIControl* uiControl = GetUIControl();
      if (uiControl)
      {
         int parentX,parentY;
         mUIControlPathEntry->GetParent()->GetPosition(parentX,parentY);
         ofPushMatrix();
         ofTranslate(-parentX, -parentY);
         ofPushStyle();
         if (mUIControlPathEntry == IKeyboardFocusListener::GetActiveKeyboardFocus())
            ofSetLineWidth(3);
         else
            ofSetLineWidth(1);
         ofSetColor(255,255,255,200);
         int pathX,pathY,pathW,pathH;
         int targetX,targetY,targetW,targetH;
         mUIControlPathEntry->GetPosition(pathX, pathY);
         mUIControlPathEntry->GetDimensions(pathW, pathH);
         uiControl->GetPosition(targetX, targetY);
         uiControl->GetDimensions(targetW, targetH);
         ofLine(pathX+pathW,pathY+pathH/2,targetX+targetW/2,targetY+targetH/2);
         ofPopStyle();
         ofPopMatrix();
      }
   }*/
}

void UIControlConnection::DrawLayout()
{
   PreDraw();
   
   mControlEntry->DrawLabel(true);
   mChannelDropdown->DrawLabel(true);
   mValueEntry->DrawLabel(true);
   mMidiOffEntry->DrawLabel(true);
   mMidiOnEntry->DrawLabel(true);
   mIncrementalEntry->DrawLabel(true);
   mFeedbackDropdown->DrawLabel(true);
   
   mMessageTypeDropdown->SetPosition(kLayoutControlsX+5, kLayoutControlsY+3);
   mControlEntry->PositionTo(mMessageTypeDropdown, kAnchor_Right_Padded);
   mChannelDropdown->PositionTo(mControlEntry, kAnchor_Right_Padded);
   mUIControlPathEntry->PositionTo(mMessageTypeDropdown, kAnchor_Below);
   mControlTypeDropdown->PositionTo(mUIControlPathEntry, kAnchor_Below);
   mValueEntry->PositionTo(mControlTypeDropdown, kAnchor_Right);
   mMidiOffEntry->PositionTo(mControlTypeDropdown, kAnchor_Below);
   mMidiOnEntry->PositionTo(mMidiOffEntry, kAnchor_Right_Padded);
   mScaleOutputCheckbox->PositionTo(mMidiOnEntry, kAnchor_Right_Padded);
   mBlinkCheckbox->PositionTo(mMidiOffEntry, kAnchor_Below);
   mIncrementalEntry->PositionTo(mBlinkCheckbox, kAnchor_Right_Padded);
   mTwoWayCheckbox->PositionTo(mBlinkCheckbox, kAnchor_Below);
   mFeedbackDropdown->PositionTo(mTwoWayCheckbox, kAnchor_Right_Padded);
   mPagelessCheckbox->PositionTo(mTwoWayCheckbox, kAnchor_Below);
   mRemoveButton->PositionTo(mPagelessCheckbox, kAnchor_Below);
   mCopyButton->SetShowing(false);
   
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
      (*iter)->Draw();
}

void UIControlConnection::SetNext(UIControlConnection* next)
{
   mControlEntry->SetNextTextEntry(next ? next->mControlEntry : nullptr);
   mUIControlPathEntry->SetNextTextEntry(next ? next->mUIControlPathEntry : nullptr);
   mValueEntry->SetNextTextEntry(next ? next->mValueEntry : nullptr);
   mMidiOffEntry->SetNextTextEntry(next ? next->mMidiOffEntry : nullptr);
   mMidiOnEntry->SetNextTextEntry(next ? next->mMidiOnEntry : nullptr);
   mIncrementalEntry->SetNextTextEntry(next ? next->mIncrementalEntry : nullptr);
}

bool UIControlConnection::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mUIOwner->GetLayoutControl(mControl, mMessageType).mControlCable &&
       (mPage == mUIOwner->GetPage() || mPageless) &&
       fromUserClick)
   {
      mUIControl = dynamic_cast<IUIControl*>(cableSource->GetTarget());
      return true;
   }
   return false;
}

UIControlConnection::~UIControlConnection()
{
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
   {
      mUIOwner->RemoveUIControl(*iter);
      (*iter)->Delete();
   }
   mEditorControls.clear();
}

void ControlLayoutElement::Setup(MidiController* owner, MidiMessageType type, int control, ControlDrawType drawType, bool incremental, int offVal, int onVal, bool scaleOutput, ControlType connectionType, float x, float y, float w, float h)
{
   assert(incremental == false || type == kMidiMessage_Control);  //only control type can be incremental
   
   mActive = true;
   mType = type;
   mControl = control;
   mDrawType = drawType;
   mIncremental = incremental;
   mOffVal = offVal;
   mOnVal = onVal;
   mScaleOutput = scaleOutput;
   mConnectionType = connectionType;
   mPosition.set(x,y);
   mDimensions.set(w,h);
   mLastValue = 0;
   mLastActivityTime = -9999;
   
   if (mControlCable == nullptr)
   {
      mControlCable = new PatchCableSource(owner, kConnectionType_UIControl);
      owner->AddPatchCableSource(mControlCable);
      ofColor color = mControlCable->GetColor();
      color.a *= .2f;
      mControlCable->SetColor(color);
   }
}
