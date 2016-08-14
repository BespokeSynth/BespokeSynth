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

bool UIControlConnection::sDrawCables = true;

MidiController::MidiController()
: mDevice(this)
, mUseNegativeEdge(false)
, mSlidersDefaultToIncremental(false)
, mBindMode(false)
, mBindCheckbox(NULL)
, mTwoWay(true)
, mControllerIndex(-1)
, mLastActivityTime(-9999)
, mLastBindControllerTime(-9999)
, mBlink(false)
, mControllerPage(0)
, mPageSelector(NULL)
, mPrintInput(false)
, mNonstandardController(NULL)
, mControllerList(NULL)
, mIsConnected(false)
, mHasCreatedConnectionUIControls(false)
{
   mListeners.resize(MAX_MIDI_PAGES);
   
   TheTransport->AddAudioPoller(this);
}

void MidiController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mBindCheckbox = new Checkbox(this,"bind",5,2,&mBindMode);
   mPageSelector = new DropdownList(this,"page",48,2,&mControllerPage);
   mAddConnectionCheckbox = new ClickButton(this,"add",12,300);
   mControllerList = new DropdownList(this,"controller",102,2,&mControllerIndex);
   //mDrawCablesCheckbox = new Checkbox(this,"draw cables",200,26,&UIControlConnection::sDrawCables);
   
   for (int i=0; i<MAX_MIDI_PAGES; ++i)
      mPageSelector->AddLabel(("page "+ofToString(i)).c_str(), i);
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

void MidiController::AddControlConnection(MidiMessageType messageType, int control, int channel, IUIControl* uicontrol)
{
   if (uicontrol)
   {
      UIControlConnection* connection = new UIControlConnection();
      connection->mMessageType = messageType;
      connection->mControl = control;
      connection->mUIControl = uicontrol;
      connection->mChannel = channel;
      connection->mPage = mControllerPage;
      if (mSlidersDefaultToIncremental)
         connection->mIncrementAmount = 1;
      connection->CreateUIControls(this, mConnections.size());
      mConnections.push_back(connection);
      uicontrol->AddRemoteController();
   }
}

void MidiController::AddControlConnection(const ofxJSONElement& connection)
{
   int control = connection["control"].asInt() % MIDI_PAGE_WIDTH;
   string path = connection["uicontrol"].asString();
   string type = connection["type"].asString();
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
   
   UIControlConnection* controlConnection = new UIControlConnection();
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
   
   if (controlConnection->mUIControl)
      controlConnection->mUIControl->AddRemoteController();
   
   if (!connection["pages"].isNull())
   {
      for (int i=0; i<connection["pages"].size(); ++i)
      {
         IUIControl* uicontrolNextPage = TheSynth->FindUIControl(connection["pages"][i].asCString());
         if (uicontrolNextPage)
         {
            UIControlConnection* nextPageConnection = new UIControlConnection(*controlConnection);
            nextPageConnection->mPage += i+1;
            nextPageConnection->mUIControl = uicontrolNextPage;
            nextPageConnection->mEditorControls.clear(); //TODO(Ryan) temp fix
            nextPageConnection->CreateUIControls(this, mConnections.size());
            mConnections.push_back(nextPageConnection);
            uicontrolNextPage->AddRemoteController();
         }
      }
   }
}

void MidiController::OnTransportAdvanced(float amount)
{
   Profiler profiler("MidiController");
   
   mMutex.lock();
   
   for (auto note = mQueuedNotes.begin(); note != mQueuedNotes.end(); ++note)
   {
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
   
   mMutex.unlock();
}

void MidiController::OnMidiNote(MidiNote& note)
{
   if (!mEnabled)
      return;
   
   MidiReceived(kMidiMessage_Note, note.mPitch, note.mVelocity/127.0f, note.mChannel);
   
   mMutex.lock();
   mQueuedNotes.push_back(note);
   mMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " note: " << note.mPitch << ", " << note.mVelocity;
}

void MidiController::OnMidiControl(MidiControl& control)
{
   if (!mEnabled)
      return;
   
   MidiReceived(kMidiMessage_Control, control.mControl, control.mValue/127.0f, control.mChannel);
   
   mMutex.lock();
   mQueuedControls.push_back(control);
   mMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " control: " << control.mControl << ", " << control.mValue;
}

void MidiController::OnMidiProgramChange(MidiProgramChange& program)
{
   if (!mEnabled)
      return;
   
   MidiReceived(kMidiMessage_Program, program.mProgram, program.mChannel);
   
   mMutex.lock();
   mQueuedProgramChanges.push_back(program);
   mMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " program change: " << program.mProgram;
}

void MidiController::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (!mEnabled)
      return;
   
   MidiReceived(kMidiMessage_PitchBend, MIDI_PITCH_BEND_CONTROL_NUM, pitchBend.mValue/16383.0f, pitchBend.mChannel);   //16383 = max pitch bend
 
   mMutex.lock();
   mQueuedPitchBends.push_back(pitchBend);
   mMutex.unlock();
   
   if (mPrintInput)
      ofLog() << Name() << " pitch bend: " << pitchBend.mValue;
}

void MidiController::MidiReceived(MidiMessageType messageType, int control, float value, int channel)
{
   assert(mEnabled);
   
   mLastActivityBound = false;
   //if (value > 0)
      mLastActivityTime = gTime;
   
   if (gTime - mLastBindControllerTime < 500)   //no midi messages if we just bound something, to avoid changing that thing we just bound
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
      mLastBindControllerTime = gTime;
      gBindToUIControl = NULL;
      return;
   }

   for (auto i=mConnections.begin(); i != mConnections.end(); ++i)
   {
      UIControlConnection* connection = *i;
      int testControl = control;
      if (connection->mMessageType == messageType &&
          connection->mControl == testControl &&
          (connection->mPageless || connection->mPage == mControllerPage) &&
          (connection->mChannel == -1 || connection->mChannel == channel))
      {
         mLastActivityBound = true;
         //if (value > 0)
            connection->mLastActivityTime = gTime;
         
         IUIControl* uicontrol = connection->GetUIControl();
         if (uicontrol == NULL)
            continue;
         
         if (connection->mType == kControlType_Slider)
         {
            if (connection->mIncrementAmount != 0)
            {
               float curValue = uicontrol->GetMidiValue();
               float increment = connection->mIncrementAmount / 100;
               if (GetKeyModifiers() & kModifier_Shift)
                  increment /= 50;
               if (value > .5f)
                  curValue += increment;
               else
                  curValue -= increment;
               uicontrol->SetFromMidiCC(curValue);
            }
            else
            {
               if (connection->mMessageType == kMidiMessage_Note)
                  value = value>0 ? 1 : 0;
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
      }
   }
   
   /*if (mLastActivityBound == false && mTwoWay) //if this didn't affect anything, give that feedback on the controller by keeping it at zero
   {
      //if (messageType == kMidiMessage_Note)
      //   SendNote(control, 0);
      if (messageType == kMidiMessage_Control)
         SendCC(mControllerPage, control, 0);
   }*/
}

void MidiController::RemoveConnection(int control, MidiMessageType messageType, int channel, int page)
{
   IUIControl* removed = NULL;
   
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
   mBlink = int(TheTransport->GetMeasurePos() * TheTransport->GetTimeSigTop() * 2) % 2 == 0;
   
   if (IsConnected())
   {
      if (!mIsConnected)
      {
         for (auto i = mListeners[mControllerPage].begin(); i != mListeners[mControllerPage].end(); ++i)
            (*i)->ControllerPageSelected();
      }
      mIsConnected = true;
   }
   else
   {
      mIsConnected = false;
      if (mNonstandardController)
      {
         if (mNonstandardController->Reconnect())
            ResyncTwoWay();
      }
      else
      {
         if (mDevice.Reconnect())
            ResyncTwoWay();
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
         if (uicontrol == NULL)
            continue;
         
         if (connection->mPage != -1 && connection->mPage != mControllerPage)
            continue;
         
         int control = connection->mControl;
         
         if (connection->mFeedbackControl != -1) // "self"
            control = connection->mFeedbackControl;
         
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
               if (connection->mMessageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, outVal, true, connection->mChannel);
               else if (connection->mMessageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, outVal, connection->mChannel);
            }
            else if (connection->mType == kControlType_Slider)
            {
               int outVal = curValue;
               if (connection->mMidiOnValue != 127 ||
                   connection->mMidiOffValue != 0) //uses defined slider range for output
               {
                  outVal = int((curValue / 127.0f) * (connection->mMidiOnValue - connection->mMidiOffValue) + connection->mMidiOffValue);
               }
               if (connection->mMessageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, outVal, true, connection->mChannel);
               else if (connection->mMessageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, outVal, connection->mChannel);
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
               if (connection->mMessageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, outVal, true, connection->mChannel);
               else if (connection->mMessageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, outVal, connection->mChannel);
            }
            else if (connection->mType == kControlType_Direct)
            {
               curValue = uicontrol->GetValue();
               if (connection->mMessageType == kMidiMessage_Note)
                  SendNote(mControllerPage, control, uicontrol->GetValue(), true, connection->mChannel);
               else if (connection->mMessageType == kMidiMessage_Control)
                  SendCC(mControllerPage, control, uicontrol->GetValue(), connection->mChannel);
            }
            connection->mLastControlValue = curValue;
         }
      }
   }
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
         mDevice.SendNote(connection->mControl, 0, connection->mChannel);
   }
}

void MidiController::SetPage(int page)
{
   mControllerPage = page;
   ResyncTwoWay();
}

void MidiController::DrawModule()
{
   if (gTime - mLastActivityTime < 200)
   {
      ofPushStyle();
      if (mLastActivityBound)
         ofSetColor(0,255,0,255*(1-(gTime - mLastActivityTime)/200));
      else
         ofSetColor(255,0,0,255*(1-(gTime - mLastActivityTime)/200));
      ofFill();
      ofRect(25+GetStringWidth(Name()),-11,10,10);
      ofPopStyle();
   }
   
   if (!mIsConnected)
   {
      float xStart = 25+GetStringWidth(Name());
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
         connection->CreateUIControls(this, i);
         ++i;
      }
      mHasCreatedConnectionUIControls = true;
   }
   
   mBindCheckbox->Draw();
   mPageSelector->Draw();
   mControllerList->Draw();
   //mDrawCablesCheckbox->Draw();
   
   int w,h;
   GetDimensions(w, h);
   mAddConnectionCheckbox->SetY(h-17);
   mAddConnectionCheckbox->Draw();
   
   DrawText("last input: "+mLastInput,60,h-5);
   
   if (gTime - mLastActivityTime < 200)
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
   
   DrawText("                                                                                                                                                                MIDI out                           all", 12, 24);
   DrawText("midi       num    chan   path                                                              type         value       inc         feedback    off    on    blink  pages", 12, 36);
   
   list<UIControlConnection*> toDraw;
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
      UIControlConnection* next = NULL;
      if (iter != toDraw.end())
         next = *iter;
      connection->SetNext(next);
      connection->Draw(i);
      ++i;
   }
   
   ofPushStyle();
   if (mIsConnected)
   {
      ofSetColor(ofColor::green);
      DrawText("connected", 300, 14);
   }
   else
   {
      ofSetColor(ofColor::red);
      DrawText("not connected", 300, 14);
   }
   ofPopStyle();
}

bool MidiController::IsConnected()
{
   if (mNonstandardController != NULL)
      return mNonstandardController->IsConnected();
   return mDevice.IsConnected();
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
            mDevice.SendNote(connection->mControl, 0, connection->mChannel);
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

void MidiController::GetModuleDimensions(int& width, int& height)
{
   width = 775;
   height = 65 + 20 * GetNumConnectionsOnPage(mControllerPage);
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
   if (page == mControllerPage)
   {
      mDevice.SendNote(pitch,velocity,forceNoteOn, channel);
      
      if (mNonstandardController)
         mNonstandardController->SendValue(page, pitch, velocity/127.0f, forceNoteOn, channel);
   }
}

void MidiController::SendCC(int page, int ctl, int value, int channel /*= -1*/)
{
   if (page == mControllerPage)
      mDevice.SendCC(ctl,value, channel);
}

void MidiController::SendData(int page, unsigned char a, unsigned char b, unsigned char c)
{
   if (page == mControllerPage)
   {
      mDevice.SendData(a,b,c);
   }
}

void MidiController::BuildControllerList()
{
   mControllerList->Clear();
   const StringArray input = MidiInput::getDevices();
   for (int i=0; i<input.size(); ++i)
      mControllerList->AddLabel(input[i].toRawUTF8(), i);
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
   return NULL;
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
   if (button == mAddConnectionCheckbox)
   {
      UIControlConnection* connection = new UIControlConnection();
      connection->mPage = mControllerPage;
      connection->CreateUIControls(this, mConnections.size());
      mConnections.push_back(connection);
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
         copy->CreateUIControls(this, mConnections.size());
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
      HighlightPageControls(mControllerPage);
      ResyncTwoWay();
   }
   if (list == mControllerList)
   {
      mDevice.ConnectInput(mControllerIndex);
      mDevice.ConnectOutput(mControllerList->GetLabel(mControllerIndex).c_str());
      mModuleSaveData.SetString("devicein", mControllerList->GetLabel(mControllerIndex));
      mModuleSaveData.SetString("deviceout", mControllerList->GetLabel(mControllerIndex));
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
            connection->mUIControl = gBindToUIControl;
            gBindToUIControl->AddRemoteController();
            gBindToUIControl = NULL;
            TextEntry::ClearActiveTextEntry(!K(acceptEntry));
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
}

void MidiController::PostRepatch(PatchCableSource* cable)
{
   for (auto* connection : mConnections)
      connection->PostRepatch(cable);
}

namespace {
   void FillMidiInput(DropdownList* list)
   {
      assert(list);
      const StringArray input = MidiInput::getDevices();
      for (int i=0; i<input.size(); ++i)
         list->AddLabel(input[i].toRawUTF8(), i);
   }
   void FillMidiOutput(DropdownList* list)
   {
      assert(list);
      const StringArray input = MidiOutput::getDevices();
      for (int i=0; i<input.size(); ++i)
         list->AddLabel(input[i].toRawUTF8(), i);
   }
}

void MidiController::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("devicein", moduleInfo, "", FillMidiInput);
   mModuleSaveData.LoadString("deviceout", moduleInfo, "", FillMidiOutput);
   mModuleSaveData.LoadInt("outchannel", moduleInfo, 1, 0, 15);
   
   mModuleSaveData.LoadBool("negativeedge",moduleInfo,false);
   mModuleSaveData.LoadBool("incrementalsliders", moduleInfo, false);
   mModuleSaveData.LoadInt("start_page",moduleInfo,0,0,MAX_MIDI_PAGES-1);
   
   mConnectionsJson = moduleInfo["connections"];

   SetUpFromSaveData();
   
   if (mNonstandardController)
      mNonstandardController->LoadInfo(moduleInfo);
}

void MidiController::SetUpFromSaveData()
{
   mDeviceIn = mModuleSaveData.GetString("devicein");
   mDeviceOut = mModuleSaveData.GetString("deviceout");
   mOutChannel = mModuleSaveData.GetInt("outchannel");
   assert(mOutChannel > 0 && mOutChannel <= 16);
   
   if (mDeviceIn == "xboxcontroller")
   {
      //TODO_PORT(Ryan)
      
      //Xbox360Controller* xbox = new Xbox360Controller(this);
      //mNonstandardController = xbox;
   }
   else if (mDeviceIn == "monome")
   {
      Monome* monome = new Monome(this);
      mNonstandardController = monome;
   }
   else if (mDeviceIn == "osccontroller")
   {
      OscController* osc = new OscController(this,"192.168.1.128",9000,8000);
      mNonstandardController = osc;
   }
   else
   {
      mDevice.ConnectInput(mDeviceIn.c_str());
   }
   
   if (mDeviceOut.length() > 0 && mNonstandardController == NULL)
   {
      mTwoWay = true;
      mDevice.ConnectOutput(mDeviceOut.c_str(), mOutChannel);
   }
   
   UseNegativeEdge(mModuleSaveData.GetBool("negativeedge"));
   mSlidersDefaultToIncremental = mModuleSaveData.GetBool("incrementalsliders");
   SetPage(mModuleSaveData.GetInt("start_page"));
   
   BuildControllerList();
   
   const std::vector<string>& devices = mDevice.GetPortList();
   for (int i=0; i<devices.size(); ++i)
   {
      if (devices[i].c_str() == mDeviceIn)
         mControllerIndex = i;
   }
}

void MidiController::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   if (dynamic_cast<OscController*>(mNonstandardController) == NULL) //TODO(Ryan)
   {
      mConnectionsJson.clear();
      mConnectionsJson.resize(mConnections.size());
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
   }
   
   moduleInfo["connections"] = mConnectionsJson;
}

void UIControlConnection::SetUIControl(string path)
{
   if (mUIControl)
      mUIControl->RemoveRemoteController();
      
   if (path == "hover")
   {
      mUIControl = NULL;
      mSpecialBinding = kSpecialBinding_Hover;
   }
   else if (path.substr(0,7) == "hotbind")
   {
      mUIControl = NULL;
      int index = path[7] - '0';
      mSpecialBinding = SpecialControlBinding(int(kSpecialBinding_HotBind0) + index);
   }
   else
   {
      mUIControl = NULL;
      try
      {
         mUIControl = TheSynth->FindUIControl(path);
      }
      catch (exception e)
      {
      }
      mSpecialBinding = kSpecialBinding_None;
      
      if (mUIControl)
         mUIControl->AddRemoteController();
   }
}

void UIControlConnection::CreateUIControls(MidiController* owner, int index)
{
   assert(mEditorControls.empty());
   
   mUIOwner = owner;
   
   int x = 12;
   int y = 42 + 20 * index;
   static int sControlID = 0;
   mMessageTypeDropdown = new DropdownList(owner,("messagetype"+ofToString(sControlID)).c_str(),x,y,((int*)&mMessageType));
   mControlEntry = new TextEntry(owner,("control"+ofToString(sControlID)).c_str(),x+47,y,3,&mControl,0,127);
   mChannelDropdown = new DropdownList(owner,("channel"+ofToString(sControlID)).c_str(),x+80,y,&mChannel);
   mUIControlPathEntry = new TextEntry(owner,("path"+ofToString(sControlID)).c_str(),x+120,y,25,mUIControlPathInput);
   mControlTypeDropdown = new DropdownList(owner,("controltype"+ofToString(sControlID)).c_str(),x+348,y,((int*)&mType));
   mValueEntry = new TextEntry(owner,("value"+ofToString(sControlID)).c_str(),x+405,y,5,&mValue,-FLT_MAX,FLT_MAX);
   mIncrementalEntry = new TextEntry(owner,("increment"+ofToString(sControlID)).c_str(),x+457,y,4,&mIncrementAmount,-FLT_MAX,FLT_MAX);
   mTwoWayCheckbox = new Checkbox(owner,("twoway"+ofToString(sControlID)).c_str(),x+505,y,&mTwoWay);
   mFeedbackDropdown = new DropdownList(owner,("feedback"+ofToString(sControlID)).c_str(),x+520,y,&mFeedbackControl);
   mMidiOffEntry = new TextEntry(owner,("midi off"+ofToString(sControlID)).c_str(),x+560,y,3,&mMidiOffValue,0,127);
   mMidiOnEntry = new TextEntry(owner,("midi on"+ofToString(sControlID)).c_str(),x+590,y,3,&mMidiOnValue,0,127);
   mBlinkCheckbox = new Checkbox(owner,("blink"+ofToString(sControlID)).c_str(),x+637,y,&mBlink);
   mPagelessCheckbox = new Checkbox(owner,("pageless"+ofToString(sControlID)).c_str(),x+670,y,&mPageless);
   mRemoveButton = new ClickButton(owner," x ",x+700,y);
   mCopyButton = new ClickButton(owner,"copy",x+720,y);
   mControlCable = new PatchCableSource(owner, kConnectionType_UIControl);
   ++sControlID;
   
   mEditorControls.push_back(mMessageTypeDropdown);
   mEditorControls.push_back(mControlEntry);
   mEditorControls.push_back(mChannelDropdown);
   mEditorControls.push_back(mUIControlPathEntry);
   mEditorControls.push_back(mControlTypeDropdown);
   mEditorControls.push_back(mValueEntry);
   mEditorControls.push_back(mMidiOffEntry);
   mEditorControls.push_back(mMidiOnEntry);
   mEditorControls.push_back(mBlinkCheckbox);
   mEditorControls.push_back(mIncrementalEntry);
   mEditorControls.push_back(mTwoWayCheckbox);
   mEditorControls.push_back(mFeedbackDropdown);
   mEditorControls.push_back(mPagelessCheckbox);
   mEditorControls.push_back(mRemoveButton);
   mEditorControls.push_back(mCopyButton);
   
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
      (*iter)->SetNoHover(true);
   
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
   mChannelDropdown->AddLabel("16", 16);
   
   mControlTypeDropdown->AddLabel("slider", kControlType_Slider);
   mControlTypeDropdown->AddLabel("set", kControlType_SetValue);
   mControlTypeDropdown->AddLabel("release", kControlType_SetValueOnRelease);
   mControlTypeDropdown->AddLabel("toggle", kControlType_Toggle);
   mControlTypeDropdown->AddLabel("direct", kControlType_Direct);
   
   mFeedbackDropdown->AddLabel("self", -1);
   mFeedbackDropdown->AddLabel("none", -2);
   for (int i=0; i<=127; ++i)
      mFeedbackDropdown->AddLabel(ofToString(i).c_str(), i);
   
   mPagelessCheckbox->SetDisplayText(false);
   mTwoWayCheckbox->SetDisplayText(false);
   mBlinkCheckbox->SetDisplayText(false);
   
   owner->AddPatchCableSource(mControlCable);
}

void UIControlConnection::SetShowing(bool enabled)
{
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
      (*iter)->SetShowing(enabled);
   mControlCable->SetEnabled(sDrawCables);
}

void UIControlConnection::Draw(int index)
{
   SetShowing(true);
   
   int y = 42 + 20 * index;
   
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
      else
      {
         StringCopy(mUIControlPathInput, mUIControl->Path().c_str(), MAX_TEXTENTRY_LENGTH);
         mControlCable->SetTarget(mUIControl);
      }
      mUIControlPathEntry->SetInErrorMode(false);
   }
   else
   {
      mUIControlPathEntry->SetInErrorMode(true);
      if (PatchCable::sActivePatchCable == NULL)
         mControlCable->ClearPatchCables();
   }
   
   mControlEntry->SetShowing(mMessageType != kMidiMessage_PitchBend);
   mValueEntry->SetShowing(mType == kControlType_SetValue || mType == kControlType_SetValueOnRelease);
   
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
   {
      (*iter)->SetY(y);
      (*iter)->Draw();
   }
   
   ofRectangle rect = mUIControlPathEntry->GetRect(true);
   mControlCable->SetManualPosition(rect.x + rect.width, rect.y + rect.height/2);
   
   if (gTime - mLastActivityTime < 200)
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
   
   /*if (mUIControlPathEntry == TextEntry::GetActiveTextEntry() || TheSynth->InMidiMapMode())
   {
      IUIControl* uiControl = GetUIControl();
      if (uiControl)
      {
         int parentX,parentY;
         mUIControlPathEntry->GetParent()->GetPosition(parentX,parentY);
         ofPushMatrix();
         ofTranslate(-parentX, -parentY);
         ofPushStyle();
         if (mUIControlPathEntry == TextEntry::GetActiveTextEntry())
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

void UIControlConnection::SetNext(UIControlConnection* next)
{
   mControlEntry->SetNextTextEntry(next ? next->mControlEntry : NULL);
   mUIControlPathEntry->SetNextTextEntry(next ? next->mUIControlPathEntry : NULL);
   mValueEntry->SetNextTextEntry(next ? next->mValueEntry : NULL);
   mMidiOffEntry->SetNextTextEntry(next ? next->mMidiOffEntry : NULL);
   mMidiOnEntry->SetNextTextEntry(next ? next->mMidiOnEntry : NULL);
   mIncrementalEntry->SetNextTextEntry(next ? next->mIncrementalEntry : NULL);
}

void UIControlConnection::PostRepatch(PatchCableSource* cable)
{
   if (cable == mControlCable)
      mUIControl = dynamic_cast<IUIControl*>(cable->GetTarget());
}

UIControlConnection::~UIControlConnection()
{
   for (auto iter = mEditorControls.begin(); iter != mEditorControls.end(); ++iter)
   {
      mUIOwner->RemoveUIControl(*iter);
      (*iter)->Delete();
   }
   mEditorControls.clear();
   if (mControlCable && mUIOwner)
      mUIOwner->RemovePatchCableSource(mControlCable);
}
