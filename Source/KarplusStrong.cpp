//
//  KarplusStrong.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "KarplusStrong.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

KarplusStrong::KarplusStrong()
: mFilterSlider(NULL)
, mFeedbackSlider(NULL)
, mVolSlider(NULL)
, mSourceDropdown(NULL)
, mMuteCheckbox(NULL)
, mStretchCheckbox(NULL)
, mCarrierSlider(NULL)
, mPolyMgr(this)
{
   mVoiceParams.mFilter = .6f;
   mVoiceParams.mVol = 1.0f;
   mVoiceParams.mFeedback = .998f;
   mVoiceParams.mSourceType = kSourceTypeMix;
   mVoiceParams.mMute = true;
   mVoiceParams.mStretch = false;
   mVoiceParams.mCarrier = 100;
   mVoiceParams.mExcitation = 0;

   mPolyMgr.Init(kVoiceType_Karplus, &mVoiceParams);

   mWriteBuffer = new float[gBufferSize];

   AddChild(&mBiquad);
   mBiquad.SetPosition(150,15);
   mBiquad.SetEnabled(true);
   mBiquad.SetFilterType(kFilterType_Lowpass);
   mBiquad.SetFilterParams(1600, 1);
   mBiquad.SetName("biquad");
}

void KarplusStrong::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFilterSlider = new FloatSlider(this,"filter",4,38,140,15,&mVoiceParams.mFilter,0,1.2f);
   mFeedbackSlider = new FloatSlider(this,"feedback",4,54,140,15,&mVoiceParams.mFeedback,.9f,.9999f,4);
   mVolSlider = new FloatSlider(this,"vol",4,22,80,15,&mVoiceParams.mVol,0,2);
   mSourceDropdown = new DropdownList(this,"source type",48,70,(int*)&mVoiceParams.mSourceType);
   mMuteCheckbox = new Checkbox(this,"mute",4,70,&mVoiceParams.mMute);
   mStretchCheckbox = new Checkbox(this,"stretch",100,22,&mVoiceParams.mStretch);
   mCarrierSlider = new FloatSlider(this,"c",95,70,50,15,&mVoiceParams.mCarrier,10,400);
   
   mSourceDropdown->AddLabel("sin", kSourceTypeSin);
   mSourceDropdown->AddLabel("white", kSourceTypeNoise);
   mSourceDropdown->AddLabel("mix", kSourceTypeMix);
   mSourceDropdown->AddLabel("saw", kSourceTypeSaw);
   
   mBiquad.CreateUIControls();
}

KarplusStrong::~KarplusStrong()
{
   delete[] mWriteBuffer;
}

void KarplusStrong::Process(double time)
{
   Profiler profiler("KarplusStrong");

   if (!mEnabled || GetTarget() == NULL)
      return;

   ComputeSliders(0);

   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);

   Clear(mWriteBuffer, gBufferSize);
   mPolyMgr.Process(time, mWriteBuffer, bufferSize);

   mBiquad.ProcessAudio(time, mWriteBuffer, bufferSize);

   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize);

   Add(out, mWriteBuffer, gBufferSize);
}

void KarplusStrong::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (velocity > 0)
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, pitchBend, modWheel, pressure);
   else
      mPolyMgr.Stop(time, pitch);
}

void KarplusStrong::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void KarplusStrong::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mFilterSlider->Draw();
   mFeedbackSlider->Draw();
   mVolSlider->Draw();
   mSourceDropdown->Draw();
   mMuteCheckbox->Draw();
   
   mStretchCheckbox->Draw();
   mCarrierSlider->Draw();

   mBiquad.Draw();
}

void KarplusStrong::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void KarplusStrong::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void KarplusStrong::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void KarplusStrong::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}


