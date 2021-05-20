//
//  SeaOfGrain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/8/14.
//
//

#include "SeaOfGrain.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "ModulationChain.h"

const float mBufferX = 5;
const float mBufferY = 100;
const float mBufferW = 800;
const float mBufferH = 200;

SeaOfGrain::SeaOfGrain()
: mVolume(.6f)
, mVolumeSlider(nullptr)
, mSample(nullptr)
, mLoading(false)
, mDisplayOffset(0)
, mDisplayOffsetSlider(nullptr)
, mDisplayLength(10)
, mDisplayLengthSlider(nullptr)
, mKeyboardBasePitch(36)
, mKeyboardBasePitchSelector(nullptr)
, mKeyboardNumPitches(24)
, mKeyboardNumPitchesSelector(nullptr)
, mDisplayStartSamples(0)
, mDisplayEndSamples(0)
{
   mSample = new Sample();
   
   for (int i=0; i<kNumMPEVoices; ++i)
      mMPEVoices[i].mOwner = this;
   
   for (int i=0; i<kNumManualVoices; ++i)
      mManualVoices[i].mOwner = this;
}

void SeaOfGrain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLoadButton = new ClickButton(this, "load", 5, 3);
   mVolumeSlider = new FloatSlider(this,"volume",5,20,150,15,&mVolume,0,2);
   mDisplayOffsetSlider = new FloatSlider(this,"offset", 5,40,150,15,&mDisplayOffset,0,10);
   mDisplayLengthSlider = new FloatSlider(this,"display length", 5,60,150,15,&mDisplayLength,1,10);
   mKeyboardBasePitchSelector = new DropdownList(this,"keyboard base pitch", 5, 80, &mKeyboardBasePitch, 60);
   mKeyboardNumPitchesSelector = new DropdownList(this,"keyboard num pitches", mKeyboardBasePitchSelector, kAnchor_Right, &mKeyboardNumPitches);
   
   mKeyboardBasePitchSelector->AddLabel("0", 0);
   mKeyboardBasePitchSelector->AddLabel("12", 12);
   mKeyboardBasePitchSelector->AddLabel("24", 24);
   mKeyboardBasePitchSelector->AddLabel("36", 36);
   mKeyboardBasePitchSelector->AddLabel("48", 48);
   mKeyboardBasePitchSelector->AddLabel("60", 60);
   
   mKeyboardNumPitchesSelector->AddLabel("12", 12);
   mKeyboardNumPitchesSelector->AddLabel("24", 24);
   mKeyboardNumPitchesSelector->AddLabel("36", 36);
   mKeyboardNumPitchesSelector->AddLabel("48", 48);
   mKeyboardNumPitchesSelector->AddLabel("60", 60);
   
   for (int i=0; i<kNumManualVoices; ++i)
   {
      float x = 10 + i * 130;
      mManualVoices[i].mGainSlider = new FloatSlider(this,("gain "+ofToString(i+1)).c_str(),x,mBufferY+mBufferH+12,120,15,&mManualVoices[i].mGain,0,1);
      mManualVoices[i].mPositionSlider = new FloatSlider(this,("pos "+ofToString(i+1)).c_str(),mManualVoices[i].mGainSlider,kAnchor_Below,120,15,&mManualVoices[i].mPosition,0,1);
      mManualVoices[i].mOverlapSlider = new FloatSlider(this,("overlap "+ofToString(i+1)).c_str(),mManualVoices[i].mPositionSlider,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mGrainOverlap,.25,MAX_GRAINS);
      mManualVoices[i].mSpeedSlider = new FloatSlider(this,("speed "+ofToString(i+1)).c_str(),mManualVoices[i].mOverlapSlider,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mSpeed,-3,3);
      mManualVoices[i].mLengthMsSlider = new FloatSlider(this,("len ms "+ofToString(i+1)).c_str(),mManualVoices[i].mSpeedSlider,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mGrainLengthMs,1,1000);
      mManualVoices[i].mPosRandomizeSlider = new FloatSlider(this,("pos r "+ofToString(i+1)).c_str(),mManualVoices[i].mLengthMsSlider,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mPosRandomizeMs,0,200);
      mManualVoices[i].mSpeedRandomizeSlider = new FloatSlider(this,("speed r "+ofToString(i+1)).c_str(),mManualVoices[i].mPosRandomizeSlider,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mSpeedRandomize,0,.3f);
      mManualVoices[i].mSpacingRandomizeSlider = new FloatSlider(this,("spacing r "+ofToString(i+1)).c_str(),mManualVoices[i].mSpeedRandomizeSlider,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mSpacingRandomize,0,1);
      mManualVoices[i].mOctaveCheckbox = new Checkbox(this,("octaves "+ofToString(i+1)).c_str(),mManualVoices[i].mSpacingRandomizeSlider,kAnchor_Below,&mManualVoices[i].mGranulator.mOctaves);
      mManualVoices[i].mWidthSlider = new FloatSlider(this,("width "+ofToString(i+1)).c_str(),mManualVoices[i].mOctaveCheckbox,kAnchor_Below,120,15,&mManualVoices[i].mGranulator.mWidth,0,1);
   }
}

SeaOfGrain::~SeaOfGrain()
{
   delete mSample;
}

void SeaOfGrain::Poll()
{
}

void SeaOfGrain::Process(double time)
{
   PROFILER(SeaOfGrain);
   
   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr || mSample == nullptr || mLoading)
      return;
   
   ComputeSliders(0);
   SyncOutputBuffer(mSample->NumChannels());
   
   int bufferSize = target->GetBuffer()->BufferSize();
   ChannelBuffer* out = target->GetBuffer();
   assert(bufferSize == gBufferSize);
   
   gWorkChannelBuffer.SetNumActiveChannels(out->NumActiveChannels());
   gWorkChannelBuffer.Clear();
   for (int i=0; i<kNumMPEVoices; ++i)
      mMPEVoices[i].Process(&gWorkChannelBuffer, bufferSize, mSample->Data());
   for (int i=0; i<kNumManualVoices; ++i)
      mManualVoices[i].Process(&gWorkChannelBuffer, bufferSize, mSample->Data());
   for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
   {
      Mult(gWorkChannelBuffer.GetChannel(ch), mVolume, bufferSize);
      GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      Add(out->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
   }
}

void SeaOfGrain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mLoadButton->Draw();
   mVolumeSlider->Draw();
   mDisplayOffsetSlider->Draw();
   mDisplayLengthSlider->Draw();
   mKeyboardBasePitchSelector->Draw();
   mKeyboardNumPitchesSelector->Draw();
   
   for (int i=0; i<kNumManualVoices; ++i)
   {
      mManualVoices[i].mGainSlider->Draw();
      mManualVoices[i].mPositionSlider->Draw();
      mManualVoices[i].mOverlapSlider->Draw();
      mManualVoices[i].mSpeedSlider->Draw();
      mManualVoices[i].mLengthMsSlider->Draw();
      mManualVoices[i].mPosRandomizeSlider->Draw();
      mManualVoices[i].mSpeedRandomizeSlider->Draw();
      mManualVoices[i].mSpacingRandomizeSlider->Draw();
      mManualVoices[i].mOctaveCheckbox->Draw();
      mManualVoices[i].mWidthSlider->Draw();
   }
   
   if (mSample)
   {
      ofPushMatrix();
      ofTranslate(mBufferX,mBufferY);
      ofPushStyle();
      
      mSample->LockDataMutex(true);
      DrawAudioBuffer(mBufferW, mBufferH, mSample->Data(), mDisplayStartSamples, mDisplayEndSamples, 0);
      mSample->LockDataMutex(false);
      
      ofPushStyle();
      ofFill();
      for (int i=0; i<mKeyboardNumPitches; ++i)
      {
         ofSetColor(i%2 * 200, 200, 0);
         ofRect(mBufferW * float(i)/mKeyboardNumPitches, mBufferH, mBufferW/mKeyboardNumPitches, 10);
      }
      ofPopStyle();
      
      for (int i=0; i<kNumMPEVoices; ++i)
         mMPEVoices[i].Draw(mBufferW, mBufferH);
      for (int i=0; i<kNumManualVoices; ++i)
         mManualVoices[i].Draw(mBufferW, mBufferH);
      
      ofPopStyle();
      ofPopMatrix();
   }
}

void SeaOfGrain::FilesDropped(vector<string> files, int x, int y)
{
   mLoading = true;
   
   mSample->Reset();
   
   mSample->Read(files[0].c_str());
   UpdateSample();
   
   mLoading = false;
}

void SeaOfGrain::SampleDropped(int x, int y, Sample* sample)
{
   mLoading = true;
   
   mSample->CopyFrom(sample);
   UpdateSample();
   
   mLoading = false;
}

void SeaOfGrain::DropdownClicked(DropdownList* list)
{
}

void SeaOfGrain::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void SeaOfGrain::UpdateSample()
{
   float sampleLengthSeconds = mSample->LengthInSamples() / mSample->GetSampleRateRatio() / gSampleRate;
   mDisplayLength = MIN(mDisplayLength,MIN(10, sampleLengthSeconds));
   mDisplayLengthSlider->SetExtents(0, sampleLengthSeconds);
   UpdateDisplaySamples();
}

void SeaOfGrain::UpdateDisplaySamples()
{
   mDisplayStartSamples = mDisplayOffset * gSampleRate * mSample->GetSampleRateRatio();
   mDisplayEndSamples = mDisplayLength * gSampleRate * mSample->GetSampleRateRatio() + mDisplayStartSamples;
}

void SeaOfGrain::LoadFile()
{
   FileChooser chooser("Load sample", File(ofToDataPath("samples")),
                       TheSynth->GetGlobalManagers()->mAudioFormatManager.getWildcardForAllFormats(), true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
   if (chooser.browseForFileToOpen())
   {
      auto file = chooser.getResult();

      vector<string> fileArray;
      fileArray.push_back(file.getFullPathName().toStdString());
      FilesDropped(fileArray, 0, 0);
   }
}

void SeaOfGrain::ButtonClicked(ClickButton *button)
{
   if (button == mLoadButton)
      LoadFile();
}

void SeaOfGrain::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void SeaOfGrain::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool SeaOfGrain::MouseMoved(float x, float y)
{
   return IDrawableModule::MouseMoved(x,y);
}

void SeaOfGrain::CheckboxUpdated(Checkbox *checkbox)
{
}

void SeaOfGrain::GetModuleDimensions(float& width, float& height)
{
   width = mBufferW + 10;
   height = mBufferY + mBufferH + 184;
}

void SeaOfGrain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mDisplayOffsetSlider || slider == mDisplayLengthSlider)
      UpdateDisplaySamples();
}

void SeaOfGrain::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void SeaOfGrain::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (voiceIdx == -1 || voiceIdx >= kNumMPEVoices)
      return;
   
   if (velocity > 0)
      mMPEVoices[voiceIdx].mADSR.Start(time, 1);
   else
      mMPEVoices[voiceIdx].mADSR.Stop(time);
   mMPEVoices[voiceIdx].mPitch = pitch;
   mMPEVoices[voiceIdx].mPlay = 0;
   mMPEVoices[voiceIdx].mPitchBend = modulation.pitchBend;
   mMPEVoices[voiceIdx].mPressure = modulation.pressure;
   mMPEVoices[voiceIdx].mModWheel = modulation.modWheel;
}

void SeaOfGrain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SeaOfGrain::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

namespace
{
   const int kSaveStateRev = 0;
}

void SeaOfGrain::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mSample->SaveState(out);
}

void SeaOfGrain::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   mSample->LoadState(in);
   UpdateSample();
}


SeaOfGrain::GrainMPEVoice::GrainMPEVoice()
: mADSR(100,0,1,100)
, mPitch(0)
, mPitchBend(nullptr)
, mPressure(nullptr)
, mModWheel(nullptr)
, mGain(0)
, mOwner(nullptr)
{
   mGranulator.mGrainLengthMs = 150;
}

void SeaOfGrain::GrainMPEVoice::Process(ChannelBuffer* output, int bufferSize, ChannelBuffer* source)
{
   if (!mADSR.IsDone(gTime) && source->BufferSize() > 0)
   {
      double time = gTime;
      for (int i=0; i< bufferSize; ++i)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(i) : 0;
         float pressure = mPressure ? mPressure->GetValue(i) : 0;
         float modwheel = mModWheel ? mModWheel->GetValue(i) : 0;
         if (pressure > 0)
         {
            mGranulator.mGrainOverlap = ofMap(pressure * pressure, 0, 1, 3, MAX_GRAINS);
            mGranulator.mPosRandomizeMs = ofMap(pressure * pressure, 0, 1, 100, .03f);
         }
         mGranulator.mGrainLengthMs = ofMap(modwheel, -1, 1, 10, 700);
         
         float blend = .0005f;
         mGain = mGain * (1-blend) + pressure * blend;

         float outSample[ChannelBuffer::kMaxNumChannels];
         Clear(outSample, ChannelBuffer::kMaxNumChannels);
         float pos = (mPitch + pitchBend + MIN(.125f, mPlay) - mOwner->mKeyboardBasePitch) / mOwner->mKeyboardNumPitches;
         mGranulator.ProcessFrame(time, source, source->BufferSize(), ofLerp(mOwner->mDisplayStartSamples, mOwner->mDisplayEndSamples, pos), outSample);
         for (int ch = 0; ch < output->NumActiveChannels(); ++ch)
            output->GetChannel(ch)[i] += outSample[ch] * sqrtf(mGain) * mADSR.Value(time);

         time += gInvSampleRateMs;
         mPlay += .001f;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }
}

void SeaOfGrain::GrainMPEVoice::Draw(float w, float h)
{
   if (!mADSR.IsDone(gTime))
   {
      if (mPitch - mOwner->mKeyboardBasePitch >= 0 && mPitch - mOwner->mKeyboardBasePitch < mOwner->mKeyboardNumPitches)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(0) : 0;
         float pressure = mPressure ? mPressure->GetValue(0) : 0;
         
         ofPushStyle();
         ofFill();
         float keyX = (mPitch - mOwner->mKeyboardBasePitch) / mOwner->mKeyboardNumPitches * w;
         float keyXTop = keyX + pitchBend * w / mOwner->mKeyboardNumPitches;
         ofBeginShape();
         ofVertex(keyX, h);
         ofVertex(keyXTop, h - pressure * h);
         ofVertex(keyXTop +10, h - pressure * h);
         ofVertex(keyX+10, h);
         ofEndShape();
         ofPopStyle();
      }
      
      mGranulator.Draw(0, 0, w, h, mOwner->mDisplayStartSamples, mOwner->mDisplayEndSamples - mOwner->mDisplayStartSamples, mOwner->mSample->LengthInSamples());
   }
}


SeaOfGrain::GrainManualVoice::GrainManualVoice()
: mGain(0)
, mPosition(0)
, mOwner(nullptr)
{
   mGranulator.mGrainLengthMs = 150;
}

void SeaOfGrain::GrainManualVoice::Process(ChannelBuffer* output, int bufferSize, ChannelBuffer* source)
{
   if (mGain > 0 && source->BufferSize() > 0)
   {
      double time = gTime;
      for (int i=0; i < bufferSize; ++i)
      {
         float outSample[ChannelBuffer::kMaxNumChannels];
         Clear(outSample, ChannelBuffer::kMaxNumChannels);
         mGranulator.ProcessFrame(time, source, source->BufferSize(), ofLerp(mOwner->mDisplayStartSamples, mOwner->mDisplayEndSamples, mPosition), outSample);
         for (int ch = 0; ch < output->NumActiveChannels(); ++ch)
            output->GetChannel(ch)[i] += outSample[ch] * mGain;
         time += gInvSampleRateMs;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }
}

void SeaOfGrain::GrainManualVoice::Draw(float w, float h)
{
   if (mGain > 0)
   {
      ofPushStyle();
      ofFill();
      float x = mPosition * w;
      float y = h - mGain * h;
      ofLine(x, y, x, h);
      ofRect(x-5, y-5, 10, 10);
      ofPopStyle();
      mGranulator.Draw(0, 0, w, h, mOwner->mDisplayStartSamples, mOwner->mDisplayEndSamples - mOwner->mDisplayStartSamples, mOwner->mSample->LengthInSamples());
   }
}



