//
//  SingleOscillator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#include "SingleOscillator.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Profiler.h"

SingleOscillator::SingleOscillator()
: mVolSlider(nullptr)
, mPhaseOffsetSlider(nullptr)
, mOscSelector(nullptr)
, mPulseWidthSlider(nullptr)
, mMult(1)
, mMultSelector(nullptr)
, mADSRDisplay(nullptr)
, mSyncCheckbox(nullptr)
, mSyncFreqSlider(nullptr)
, mDetuneSlider(nullptr)
, mUnisonSlider(nullptr)
, mUnisonWidthSlider(nullptr)
, mADSRModeSelector(nullptr)
, mADSRMode(0)
, mShuffleSlider(nullptr)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mLengthMultiplier(1)
, mLengthMultiplierSlider(nullptr)
, mWriteBuffer(gBufferSize)
, mDrawOsc(kOsc_Square)
{
   mVoiceParams.mAdsr.Set(10,0,1,10);
   mVoiceParams.mVol = .05f;
   mVoiceParams.mPulseWidth = .5f;
   mVoiceParams.mSync = false;
   mVoiceParams.mSyncFreq = 200;
   mVoiceParams.mMult = 1;
   mVoiceParams.mOscType = kOsc_Square;
   mVoiceParams.mDetune = 1;
   mVoiceParams.mFilterAdsr.Set(1,0,1,30);
   mVoiceParams.mFilterCutoff = SINGLEOSCILLATOR_NO_CUTOFF;
   mVoiceParams.mFilterQ = 1;
   mVoiceParams.mShuffle = 0;
   mVoiceParams.mPhaseOffset = 0;
   mVoiceParams.mUnison = 1;
   mVoiceParams.mUnisonWidth = 0;
   
   mPolyMgr.Init(kVoiceType_SingleOscillator, &mVoiceParams);
}

void SingleOscillator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mOscSelector = new DropdownList(this,"osc",5,1,(int*)(&mVoiceParams.mOscType));
   mOscSelector->AddLabel("sin",kOsc_Sin);
   mOscSelector->AddLabel("squ",kOsc_Square);
   mOscSelector->AddLabel("tri",kOsc_Tri);
   mOscSelector->AddLabel("saw",kOsc_Saw);
   mOscSelector->AddLabel("-saw",kOsc_NegSaw);
   mOscSelector->AddLabel("noise",kOsc_Random);
   mMultSelector = new DropdownList(this,"mult",mOscSelector,kAnchor_Right,&mMult);
   mPulseWidthSlider = new FloatSlider(this,"pw",5,55,80,15,&mVoiceParams.mPulseWidth,0.01f,.99f,2);
   mShuffleSlider = new FloatSlider(this,"shuffle",mPulseWidthSlider,kAnchor_Below,80,15,&mVoiceParams.mShuffle, 0, 1);
   mDetuneSlider = new FloatSlider(this,"detune",mShuffleSlider,kAnchor_Below,80,15,&mVoiceParams.mDetune,.95f,1.05f,3);
   mPhaseOffsetSlider = new FloatSlider(this,"phase",mDetuneSlider,kAnchor_Below,80,15,&mVoiceParams.mPhaseOffset,0,TWO_PI);
   mUnisonSlider = new IntSlider(this,"unison",mPhaseOffsetSlider,kAnchor_Below,80,15,&mVoiceParams.mUnison,1,SingleOscillatorVoice::kMaxUnison);
   mUnisonWidthSlider = new FloatSlider(this,"width",mUnisonSlider,kAnchor_Right,80,15,&mVoiceParams.mUnisonWidth,0,1);
   
   mADSRModeSelector = new RadioButton(this,"envmode",95,1,&mADSRMode,kRadioHorizontal);
   mADSRDisplay = new ADSRDisplay(this,"env",95,18,80,36,&mVoiceParams.mAdsr);
   mFilterADSRDisplay = new ADSRDisplay(this,"envfilter",95,18,80,36,&mVoiceParams.mFilterAdsr);
   mLengthMultiplierSlider = new FloatSlider(this,"len",mADSRDisplay,kAnchor_Below,80,15,&mLengthMultiplier,.01f,10);
   mVolSlider = new FloatSlider(this,"vol",mLengthMultiplierSlider,kAnchor_Below,80,15,&mVoiceParams.mVol,0,1);
   mFilterCutoffSlider = new FloatSlider(this,"f",mVolSlider,kAnchor_Below,40,15,&mVoiceParams.mFilterCutoff,0,SINGLEOSCILLATOR_NO_CUTOFF);
   mFilterQSlider = new FloatSlider(this,"q",mFilterCutoffSlider,kAnchor_Right,40,15,&mVoiceParams.mFilterQ,.1,9.99f,2);
   mSyncCheckbox = new Checkbox(this,"sync",mFilterCutoffSlider,kAnchor_Below,&mVoiceParams.mSync);
   mSyncFreqSlider = new FloatSlider(this,"syncf",mSyncCheckbox,kAnchor_Right,40,15,&mVoiceParams.mSyncFreq,10,999.9f);

   mSyncFreqSlider->SetLabel("");
   
   mFilterCutoffSlider->SetMaxValueDisplay("none");
   
   mMultSelector->AddLabel("1/8", -8);
   mMultSelector->AddLabel("1/7", -7);
   mMultSelector->AddLabel("1/6", -6);
   mMultSelector->AddLabel("1/5", -5);
   mMultSelector->AddLabel("1/4", -4);
   mMultSelector->AddLabel("1/3", -3);
   mMultSelector->AddLabel("1/2", -2);
   mMultSelector->AddLabel("1", 1);
   mMultSelector->AddLabel("1.5", -1);
   mMultSelector->AddLabel("2", 2);
   mMultSelector->AddLabel("3", 3);
   mMultSelector->AddLabel("4", 4);
   mMultSelector->AddLabel("5", 5);
   mMultSelector->AddLabel("6", 6);
   mMultSelector->AddLabel("7", 7);
   mMultSelector->AddLabel("8", 8);
   
   mADSRModeSelector->AddLabel("vol",0);
   mADSRModeSelector->AddLabel("filter",1);
   
   UpdateADSRDisplays();
   
   mWriteBuffer.SetNumActiveChannels(2);
}

SingleOscillator::~SingleOscillator()
{
}

void SingleOscillator::Process(double time)
{
   PROFILER(SingleOscillator);

   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   mNoteInputBuffer.Process(time);
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);
   
   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(GetTarget()->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void SingleOscillator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!NoteInputBuffer::IsTimeWithinFrame(time))
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
      mVoiceParams.mAdsr.Start(time,1);         //for visualization
      mVoiceParams.mFilterAdsr.Start(time,1);   //for visualization
   }
   else
   {
      mPolyMgr.Stop(time, pitch);
      mVoiceParams.mAdsr.Stop(time, false);         //for visualization
      mVoiceParams.mFilterAdsr.Stop(time, false);   //for visualization
   }
   
   if (mDrawDebug)
   {
      vector<string> lines = ofSplitString(mDebugLines, "\n");
      mDebugLines = "";
      const int kNumDisplayLines = 10;
      for (int i=0; i<kNumDisplayLines-1; ++i)
      {
         int lineIndex = (int)lines.size()-(kNumDisplayLines-1) + i;
         if (lineIndex >= 0)
            mDebugLines += lines[lineIndex] + "\n";
      }
      mDebugLines += "PlayNote("+ofToString(time)+", "+ofToString(pitch)+", "+ofToString(velocity)+", "+ofToString(voiceIdx)+")";
   }
}

void SingleOscillator::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void SingleOscillator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mPhaseOffsetSlider->Draw();
   mLengthMultiplierSlider->Draw();
   mPulseWidthSlider->Draw();
   mSyncCheckbox->Draw();
   mSyncFreqSlider->Draw();
   mOscSelector->Draw();
   mDetuneSlider->Draw();
   mUnisonSlider->Draw();
   mUnisonWidthSlider->Draw();
   mShuffleSlider->Draw();
   mFilterADSRDisplay->Draw();
   mFilterCutoffSlider->Draw();
   mFilterQSlider->Draw();
   if (mADSRMode == 0)  //draw order
   {
      mADSRModeSelector->Draw();
      mADSRDisplay->Draw();
   }
   else
   {
      mADSRDisplay->Draw();
      mADSRModeSelector->Draw();
   }
   mMultSelector->Draw();
   
   {
      int x = 5;
      int y = 18;
      int height = 36;
      int width = 80;
      
      ofSetColor(100,100,.8f*gModuleDrawAlpha);
      ofSetLineWidth(.5f);
      ofRect(x,y,width,height, 0);
      
      ofSetColor(245, 58, 0, gModuleDrawAlpha);
      ofSetLineWidth(1);
      
      ofBeginShape();
      
      for (float i=0; i<width; i+=(.25f/gDrawScale))
      {
         float phase = i/width * FTWO_PI;
         phase += gTime * .005f;
         if (mDrawOsc.GetShuffle() > 0)
            phase *= 2;
         float value = mDrawOsc.Value(phase);
         ofVertex(i + x, ofMap(value,-1,1,0,height) + y);
      }
      ofEndShape(false);
   }
}
 
void SingleOscillator::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(200, 0);
      DrawTextNormal(mDebugLines, 0, 160);
   }
}

void SingleOscillator::GetModuleDimensions(float& width, float& height)
{
   width = 180;
   height = 143;
}

void SingleOscillator::UpdateADSRDisplays()
{
   mADSRDisplay->SetActive(mADSRMode == 0);
   mFilterADSRDisplay->SetActive(mADSRMode == 1);
}

void SingleOscillator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("vol", moduleInfo, .5, mVolSlider);
   mModuleSaveData.LoadEnum<OscillatorType>("osc", moduleInfo, kOsc_Sin, mOscSelector);
   mModuleSaveData.LoadFloat("detune", moduleInfo, 1, mDetuneSlider);
   mModuleSaveData.LoadBool("pressure_envelope", moduleInfo);
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);

   SetUpFromSaveData();
}

void SingleOscillator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);
}


void SingleOscillator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mMultSelector)
   {
      if (mMult > 0)
         mVoiceParams.mMult = mMult;
      else if (mMult == -1)   //-1 is special case for 1.5
         mVoiceParams.mMult = 1.5f;
      else                    //other negative numbers mean 1/-x
         mVoiceParams.mMult = -1.0f/mMult;
   }
   if (list == mOscSelector)
      mDrawOsc.SetType(mVoiceParams.mOscType);
}

void SingleOscillator::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   if (list == mADSRModeSelector)
   {
      UpdateADSRDisplays();
   }
}

void SingleOscillator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mLengthMultiplierSlider)
   {
      mADSRDisplay->SetMaxTime(1000 * mLengthMultiplier);
      mFilterADSRDisplay->SetMaxTime(1000 * mLengthMultiplier);
   }
   if (slider == mShuffleSlider)
      mDrawOsc.SetShuffle(mVoiceParams.mShuffle);
   if (slider == mPulseWidthSlider)
      mDrawOsc.SetPulseWidth(mVoiceParams.mPulseWidth);
}

void SingleOscillator::IntSliderUpdated(IntSlider* slider, int oldVal)
{

}

void SingleOscillator::CheckboxUpdated(Checkbox* checkbox)
{
}

