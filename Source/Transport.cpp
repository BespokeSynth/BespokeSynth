//
//  Transport.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "Transport.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "ChaosEngine.h"
#include "FillSaveDropdown.h"

Transport* TheTransport = nullptr;

Transport::Transport()
: mTempo(gDefaultTempo)
, mTimeSigTop(4)
, mTimeSigBottom(4)
, mMeasureCount(0)
, mMeasurePos(0)
, mSwingInterval(8)
, mSwing(.5f)
, mSwingSlider(nullptr)
, mResetButton(nullptr)
, mTimeSigTopDropdown(nullptr)
, mTimeSigBottomDropdown(nullptr)
, mSwingIntervalDropdown(nullptr)
, mSetTempoBool(false)
, mSetTempoCheckbox(nullptr)
, mStartRecordTime(-1)
, mNudgeBackButton(nullptr)
, mNudgeForwardButton(nullptr)
, mIncreaseTempoButton(nullptr)
, mDecreaseTempoButton(nullptr)
, mTempoSlider(nullptr)
, mLoopStartMeasure(-1)
, mLoopEndMeasure(-1)
{
   assert(TheTransport == nullptr);
   TheTransport = this;

   SetName("transport");
}

void Transport::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTempoSlider = new FloatSlider(this,"tempo",5,4,93,15,&mTempo,1,225);
   mIncreaseTempoButton = new ClickButton(this," + ",118,4);
   mDecreaseTempoButton = new ClickButton(this," - ",101,4);
   mSwingSlider = new FloatSlider(this,"swing",5,22,93,15,&mSwing,.5f,.7f);
   mSwingIntervalDropdown = new DropdownList(this,"swing interval",101,22,&mSwingInterval);
   mTimeSigTopDropdown = new DropdownList(this,"timesigtop",101,42,&mTimeSigTop);
   mTimeSigBottomDropdown = new DropdownList(this,"timesigbottom",101,60,&mTimeSigBottom);
   mResetButton = new ClickButton(this, "reset",5,78);
   mNudgeBackButton = new ClickButton(this," < ",80,78);
   mNudgeForwardButton = new ClickButton(this," > ",110,78);
   mSetTempoCheckbox = new Checkbox(this,"set tempo",HIDDEN_UICONTROL,HIDDEN_UICONTROL,&mSetTempoBool);
   
   mTimeSigTopDropdown->AddLabel("2", 2);
   mTimeSigTopDropdown->AddLabel("3", 3);
   mTimeSigTopDropdown->AddLabel("4", 4);
   mTimeSigTopDropdown->AddLabel("5", 5);
   mTimeSigTopDropdown->AddLabel("6", 6);
   mTimeSigTopDropdown->AddLabel("7", 7);
   mTimeSigTopDropdown->AddLabel("8", 8);
   mTimeSigTopDropdown->AddLabel("9", 9);
   mTimeSigTopDropdown->AddLabel("10", 10);
   mTimeSigTopDropdown->AddLabel("11", 11);
   mTimeSigTopDropdown->AddLabel("12", 12);
   mTimeSigTopDropdown->AddLabel("13", 13);
   mTimeSigTopDropdown->AddLabel("14", 14);
   mTimeSigTopDropdown->AddLabel("15", 15);
   mTimeSigTopDropdown->AddLabel("16", 16);
   mTimeSigTopDropdown->AddLabel("17", 17);
   mTimeSigTopDropdown->AddLabel("18", 18);
   mTimeSigTopDropdown->AddLabel("19", 19);
   
   mTimeSigBottomDropdown->AddLabel("2", 2);
   mTimeSigBottomDropdown->AddLabel("4", 4);
   mTimeSigBottomDropdown->AddLabel("8", 8);
   mTimeSigBottomDropdown->AddLabel("16", 16);
   
   mSwingIntervalDropdown->AddLabel("4n", 4);
   mSwingIntervalDropdown->AddLabel("8n", 8);
   mSwingIntervalDropdown->AddLabel("16n", 16);
}

void Transport::Init()
{
   IDrawableModule::Init();
}

void Transport::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
}

void Transport::AdjustTempo(float amount)
{
   SetTempo(MAX(1,TheTransport->GetTempo() + amount));
}

float Transport::MsPerBar() const
{
   return 60/mTempo * 1000 * mTimeSigTop * 4.0f/mTimeSigBottom;
}

void Transport::Advance(float ms)
{
   float amount = ms/MsPerBar();
   
   assert(amount > 0);
   
   int oldMeasureCount = mMeasureCount;
   
   mMeasurePos += amount;
   while (mMeasurePos>1)
   {
      mMeasurePos -= 1;
      ++mMeasureCount;
      if (mLoopStartMeasure != -1 && (mMeasureCount < mLoopStartMeasure || mMeasureCount >= mLoopEndMeasure))
         mMeasureCount = mLoopStartMeasure;
   }
   
   if (TheChaosEngine)
      TheChaosEngine->AudioUpdate();

   UpdateListeners(ms);

   for (list<IAudioPoller*>::iterator i = mAudioPollers.begin(); i != mAudioPollers.end(); ++i)
   {
      IAudioPoller* poller = *i;
      poller->OnTransportAdvanced(amount);
   }
}

float QuadraticBezier (float x, float a, float b)
{
	// adapted from BEZMATH.PS (1993)
	// by Don Lancaster, SYNERGETICS Inc.
	// http://www.tinaja.com/text/bezmath.html
   
	float epsilon = 0.00001f;
	a = ofClamp(a,0,1);
	b = ofClamp(b,0,1);
	if (a == 0.5f){
		a += epsilon;
	}
   
	// solve t from x (an inverse operation)
	float om2a = 1 - 2*a;
	float t = (sqrtf(a*a + om2a*x) - a)/om2a;
	float y = (1-2*b)*(t*t) + (2*b)*t;
	return y;
}

float Transport::Swing(float measurePos)
{
   float swingSlices = float(mSwingInterval) * mTimeSigTop / 4.0f;
   
   float swingPos = measurePos * swingSlices;
   int swingBeat = int(swingPos);
   swingPos -= swingBeat;
   
   float swung = SwingBeat(swingPos);
   
   return (swingBeat + swung) / swingSlices;
}

float Transport::SwingBeat(float pos)
{
   float term = (.5f-mSwing) / (mSwing*mSwing - mSwing);
   pos = term*pos*pos + (1-term)*pos;
   return pos;
}

void Transport::Nudge(float amount)
{
   mMeasurePos += amount;
   if (mMeasurePos < 0)
   {
      mMeasurePos += 1;
      --mMeasureCount;
   }
   else if (mMeasurePos > 1)
   {
      mMeasurePos -= 1;
      ++mMeasureCount;
   }
}

void Transport::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   int count = int(mMeasurePos*mTimeSigTop) + 1;
   string display;
   display += ofToString(mMeasurePos,2)+" "+ofToString(mMeasureCount)+"\n";
   display += ofToString(count);
   DrawText(display,5,52);

   ofPushStyle();
   int w,h;
   GetDimensions(w,h);
   ofFill();
   ofSetColor(255,255,255,50);
   float beatWidth = w/mTimeSigTop;
   ofRect((count-1)*beatWidth,0,beatWidth,h);
   if (count % 2)
      ofSetColor(255,0,255);
   else
      ofSetColor(0,255,255);
   ofLine(w*mMeasurePos,0,w*mMeasurePos,h);
   ofRect(0,95,w*((mMeasurePos+(mMeasureCount%4))/4),5);
   ofPopStyle();

   mSwingSlider->Draw();
   mResetButton->Draw();
   mTimeSigTopDropdown->Draw();
   mTimeSigBottomDropdown->Draw();
   mSwingIntervalDropdown->Draw();
   mNudgeBackButton->Draw();
   mNudgeForwardButton->Draw();
   mIncreaseTempoButton->Draw();
   mDecreaseTempoButton->Draw();
   mTempoSlider->Draw();
   
   ofBeginShape();
   for (int i=0;i<w;++i)
   {
      float pos = i/float(w);
      float swung = Swing(pos);
      ofVertex(i,h-swung*h);
   }
   ofEndShape();
   ofRect(0,h-Swing(mMeasurePos)*h,4,1);
}

void Transport::Reset()
{
   mMeasurePos = .999f;
   mMeasureCount = -1;
}

void Transport::ButtonClicked(ClickButton *button)
{
   if (button == mResetButton)
      Reset();
   if (button == mNudgeBackButton)
      Nudge(-.03f);
   if (button == mNudgeForwardButton)
      Nudge(.03f);
   if (button == mIncreaseTempoButton)
      AdjustTempo(1);
   if (button == mDecreaseTempoButton)
      AdjustTempo(-1);
}

void Transport::AddListener(ITimeListener* listener, NoteInterval interval, float offset /*= 0*/, bool offsetIsInMs /*=true*/)
{
   //try to update first in case we already point to this
   if (!UpdateListener(listener, interval, offset, offsetIsInMs))
      mListeners.push_front(TransportListenerInfo(listener, interval, offset, offsetIsInMs));
}

bool Transport::UpdateListener(ITimeListener* listener, NoteInterval interval, float offset /*= 0*/, bool offsetIsInMs /*=true*/)
{
   for (list<TransportListenerInfo>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
   {
      TransportListenerInfo& info = *i;
      if (info.mListener == listener)
      {
         info.mInterval = interval;
         info.mOffset = offset;
         info.mOffsetIsInMs = offsetIsInMs;
         return true;
      }
   }
   return false;
}

void Transport::RemoveListener(ITimeListener* listener)
{
   for (list<TransportListenerInfo>::iterator i = mListeners.begin(); i != mListeners.end();)
   {
      TransportListenerInfo& info = *i;
      if (info.mListener == listener)
         i = mListeners.erase(i);
      else
         ++i;
   }
}

void Transport::AddAudioPoller(IAudioPoller* poller)
{
   if (!ListContains(poller, mAudioPollers))
      mAudioPollers.push_front(poller);
}

void Transport::RemoveAudioPoller(IAudioPoller* poller)
{
   mAudioPollers.remove(poller);
}

int Transport::GetQuantized(float offsetMs, NoteInterval interval)
{
   float offset = offsetMs / MsPerBar();
   float measurePos = mMeasurePos;
   while (measurePos+offset > 1) measurePos -= 1;
   while (measurePos+offset < 0) measurePos += 1;
   float pos = Swing(measurePos + offset);
   pos *= float(mTimeSigTop) / mTimeSigBottom;
   
   switch (interval)
   {
      case kInterval_1n:
      case kInterval_2: //TODO(Ryan) not really, but whatever
      case kInterval_3:
      case kInterval_4:
      case kInterval_8:
      case kInterval_16:
      case kInterval_32:
      case kInterval_64:
         return (mMeasurePos + offset < 0) || (mMeasurePos + offset >= 1) ? 1 : 0;
      case kInterval_2n:
      case kInterval_2nt:
      case kInterval_4n:
      case kInterval_4nt:
      case kInterval_8n:
      case kInterval_8nt:
      case kInterval_16n:
      case kInterval_16nt:
      case kInterval_32n:
      case kInterval_32nt:
      case kInterval_64n:
         return int(pos * CountInStandardMeasure(interval));
      case kInterval_None:
         return int(pos * CountInStandardMeasure(kInterval_16n)); //TODO(Ryan) whatever
      default:
         //TODO(Ryan) this doesn't really make sense, does it?
         assert(false);
   }
   return 0;
}

int Transport::CountInStandardMeasure(NoteInterval interval)
{
   switch (interval)
   {
      case kInterval_1n:
         return 1;
      case kInterval_2n:
         return 2;
      case kInterval_2nt:
         return 3;
      case kInterval_4n:
         return 4;
      case kInterval_4nt:
         return 6;
      case kInterval_8n:
         return 8;
      case kInterval_8nt:
         return 12;
      case kInterval_16n:
         return 16;
      case kInterval_16nt:
         return 24;
      case kInterval_32n:
         return 32;
      case kInterval_32nt:
         return 48;
      case kInterval_64n:
         return 64;
      case kInterval_None:
         return 16;  //TODO(Ryan) whatever
      default:
         //TODO(Ryan) this doesn't really make sense, does it?
         assert(false);
   }
   return 0;
}

float Transport::GetDuration(NoteInterval interval)
{
   float msPerBar = MsPerBar();
   switch (interval)
   {
      case kInterval_1n:
         return msPerBar;
      case kInterval_2n:
         return GetDuration(kInterval_4n)*2;
      case kInterval_2nt:
         return GetDuration(kInterval_2n) * 2.0f / 3.0f;
      case kInterval_4n:
         return msPerBar/mTimeSigTop;
      case kInterval_4nt:
         return GetDuration(kInterval_4n) * 2.0f / 3.0f;
      case kInterval_8n:
         return GetDuration(kInterval_4n)*.5f;
      case kInterval_8nt:
         return GetDuration(kInterval_8n) * 2.0f / 3.0f;
      case kInterval_16n:
         return GetDuration(kInterval_4n)*.25f;
      case kInterval_16nt:
         return GetDuration(kInterval_16n) * 2.0f / 3.0f;
      case kInterval_32n:
         return GetDuration(kInterval_4n)*.125f;
      case kInterval_32nt:
         return GetDuration(kInterval_32n) * 2.0f / 3.0f;
      case kInterval_64n:
         return GetDuration(kInterval_4n)*.0625f;
      case kInterval_4nd:
         return GetDuration(kInterval_4n)*1.5f;
      case kInterval_8nd:
         return GetDuration(kInterval_8n)*1.5f;
      case kInterval_16nd:
         return GetDuration(kInterval_16n)*1.5f;
      case kInterval_2:
         return msPerBar*2;
      case kInterval_3:
         return msPerBar*3;
      case kInterval_4:
         return msPerBar*4;
      case kInterval_8:
         return msPerBar*8;
      case kInterval_16:
         return msPerBar*16;
      case kInterval_32:
         return msPerBar*32;
      case kInterval_64:
         return msPerBar*64;
      default:
         return GetDuration(kInterval_4n)*.25f;
   }
}

void Transport::UpdateListeners(float jumpMs)
{
   for (list<TransportListenerInfo>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
   {
      const TransportListenerInfo& info = *i;
      if (info.mInterval != kInterval_None &&
          info.mInterval != kInterval_Free)
      {
         if (info.mOffsetIsInMs)
         {
            if (GetQuantized(info.mOffset-jumpMs, info.mInterval) != GetQuantized(info.mOffset, info.mInterval))
            {
               info.mListener->OnTimeEvent(0); //TODO(Ryan) calc sample offset
            }
         }
         else
         {
            float offsetMs = info.mOffset*MsPerBar();
            if (GetQuantized(offsetMs-jumpMs, info.mInterval) != GetQuantized(offsetMs, info.mInterval))
            {
               info.mListener->OnTimeEvent(0); //TODO(Ryan) calc sample offset
            }
         }
      }
   }
}

void Transport::OnDrumEvent(NoteInterval drumEvent)
{
   for (list<TransportListenerInfo>::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
   {
      const TransportListenerInfo& info = *i;
      if (info.mInterval == drumEvent)
         info.mListener->OnTimeEvent(0); //TODO(Ryan) calc sample offset
   }
}

float Transport::GetMeasurePos(int offset) const
{
   float inc = (offset*gInvSampleRateMs) / MsPerBar();
   float pos = mMeasurePos + inc;
   if (pos >= 1)
      return pos - 1;
   return pos;
}

void Transport::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Transport::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mSetTempoCheckbox)
   {
      if (mSetTempoBool)
      {
         mStartRecordTime = gTime;
      }
      else if (mStartRecordTime != -1)
      {
         int numBars = 1;
         float recordedTime = gTime - mStartRecordTime;
         int beats = numBars * GetTimeSigTop();
         float minutes = recordedTime / 1000.0f / 60.0f;
         TheTransport->SetTempo(beats/minutes);
         TheTransport->SetDownbeat();
      }
   }
}

void Transport::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void Transport::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void Transport::SetUpFromSaveData()
{
}

namespace
{
   const int kSaveStateRev = 0;
}

void Transport::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mMeasurePos;
}

void Transport::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mMeasurePos;
}

