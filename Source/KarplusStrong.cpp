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
, mNoteInputBuffer(this)
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
   
   mWriteBuffer.SetNumActiveChannels(2);
}

KarplusStrong::~KarplusStrong()
{
}

void KarplusStrong::Process(double time)
{
   PROFILER(KarplusStrong);

   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   mNoteInputBuffer.Process(time);

   ComputeSliders(0);

   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);

   mBiquad.ProcessAudio(time, &mWriteBuffer);

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(GetTarget()->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void KarplusStrong::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!NoteInputBuffer::IsTimeWithinFrame(time))
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
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
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);

   SetUpFromSaveData();
}

void KarplusStrong::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);
}


