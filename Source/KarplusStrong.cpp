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
: mFilterSlider(nullptr)
, mFeedbackSlider(nullptr)
, mVolSlider(nullptr)
, mSourceDropdown(nullptr)
, mMuteCheckbox(nullptr)
, mStretchCheckbox(nullptr)
, mExciterFreqSlider(nullptr)
, mExciterAttackSlider(nullptr)
, mExciterDecaySlider(nullptr)
, mPolyMgr(this)
, mWriteBuffer(gBufferSize)
{
   mPolyMgr.Init(kVoiceType_Karplus, &mVoiceParams);

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
   mVolSlider = new FloatSlider(this,"vol",3,2,80,15,&mVoiceParams.mVol,0,2);
   mMuteCheckbox = new Checkbox(this,"mute",mVolSlider,kAnchor_Right,&mVoiceParams.mMute);
   mFilterSlider = new FloatSlider(this,"filter",mVolSlider,kAnchor_Below,140,15,&mVoiceParams.mFilter,0,1.2f);
   mFeedbackSlider = new FloatSlider(this,"feedback",mFilterSlider,kAnchor_Below,140,15,&mVoiceParams.mFeedback,.9f,.9999f,4);
   mSourceDropdown = new DropdownList(this,"source type",mFeedbackSlider,kAnchor_Below,(int*)&mVoiceParams.mSourceType,45);
   mExciterFreqSlider = new FloatSlider(this,"x freq",mSourceDropdown,kAnchor_Right,92,15,&mVoiceParams.mExciterFreq,10,4000);
   mExciterAttackSlider = new FloatSlider(this,"x att",mSourceDropdown,kAnchor_Below,69,15,&mVoiceParams.mExciterAttack,1,40);
   mExciterDecaySlider = new FloatSlider(this,"x dec",mExciterAttackSlider,kAnchor_Right,68,15,&mVoiceParams.mExciterDecay,1,40);
   //mStretchCheckbox = new Checkbox(this,"stretch",mVolSlider,kAnchor_Right,&mVoiceParams.mStretch);
   
   mSourceDropdown->AddLabel("sin", kSourceTypeSin);
   mSourceDropdown->AddLabel("white", kSourceTypeNoise);
   mSourceDropdown->AddLabel("mix", kSourceTypeMix);
   mSourceDropdown->AddLabel("saw", kSourceTypeSaw);
   
   mExciterFreqSlider->SetMode(FloatSlider::kSquare);
   mExciterAttackSlider->SetMode(FloatSlider::kSquare);
   mExciterDecaySlider->SetMode(FloatSlider::kSquare);
   
   mBiquad.CreateUIControls();
}

KarplusStrong::~KarplusStrong()
{
}

void KarplusStrong::Process(double time)
{
   Profiler profiler("KarplusStrong");

   if (!mEnabled || GetTarget() == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();
   mPolyMgr.Process(time, mWriteBuffer.GetChannel(0), bufferSize);

   mBiquad.ProcessAudio(time, &mWriteBuffer);

   GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(0), bufferSize, 0);

   Add(out, mWriteBuffer.GetChannel(0), gBufferSize);
}

void KarplusStrong::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
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
   
   //mStretchCheckbox->Draw();
   mExciterFreqSlider->Draw();
   mExciterAttackSlider->Draw();
   mExciterDecaySlider->Draw();

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


