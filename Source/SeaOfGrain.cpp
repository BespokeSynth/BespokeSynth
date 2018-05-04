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
, mKeyOffset(0)
, mKeyOffsetSlider(nullptr)
, mDisplayKeys(61)
, mDisplayKeysSlider(nullptr)
, mKeyboardBaseNote(36)
, mKeyboardBaseNoteSelector(nullptr)
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   mSample = new Sample();
   
   for (float i=0; i<1; i += .01f)
      mSlices.push_back(i);
}

void SeaOfGrain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"volume",5,20,150,15,&mVolume,0,2);
   mKeyOffsetSlider = new IntSlider(this,"key offset", 5,40,150,15,&mKeyOffset,0,1000);
   mDisplayKeysSlider = new IntSlider(this,"num display keys", 5,60,150,15,&mDisplayKeys,0,200);
   mKeyboardBaseNoteSelector = new DropdownList(this,"keyboard base note", 5, 80, &mKeyboardBaseNote);
   
   mKeyboardBaseNoteSelector->AddLabel("0", 0);
   mKeyboardBaseNoteSelector->AddLabel("12", 12);
   mKeyboardBaseNoteSelector->AddLabel("24", 24);
   mKeyboardBaseNoteSelector->AddLabel("36", 36);
   mKeyboardBaseNoteSelector->AddLabel("48", 48);
   mKeyboardBaseNoteSelector->AddLabel("60", 60);
}

SeaOfGrain::~SeaOfGrain()
{
   delete[] mWriteBuffer;
   delete mSample;
}

void SeaOfGrain::Poll()
{
}

void SeaOfGrain::Process(double time)
{
   Profiler profiler("SeaOfGrain");
   
   if (!mEnabled || GetTarget() == nullptr || mSample == nullptr || mLoading)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, bufferSize);
   for (int i=0; i<NUM_SEAOFGRAIN_VOICES; ++i)
      mVoices[i].Process(mWriteBuffer, bufferSize, mSample->Data()->GetChannel(0), GetSampleLength(), mKeyOffset, mSlices);
   Mult(mWriteBuffer, mVolume, bufferSize);
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);
   
   Add(out, mWriteBuffer, bufferSize);
}

void SeaOfGrain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
   mKeyOffsetSlider->Draw();
   mDisplayKeysSlider->Draw();
   mKeyboardBaseNoteSelector->Draw();
   
   if (mSample)
   {
      ofPushMatrix();
      ofTranslate(mBufferX,mBufferY);
      ofPushStyle();
      
      int length = GetSampleLength();
      int sampleStart = mSlices[mKeyOffset] * length;
      int sampleLength = (mSlices[mKeyOffset + mDisplayKeys] - mSlices[mKeyOffset]) * length;
      sampleLength = MAX(1, sampleLength);
      
      mSample->LockDataMutex(true);
      DrawAudioBuffer(mBufferW, mBufferH, mSample->Data(), sampleStart, sampleStart + sampleLength, 0);
      mSample->LockDataMutex(false);
      
      ofPushStyle();
      ofFill();
      for (int i=0; i<mDisplayKeys; ++i)
      {
         ofSetColor(i%2 * 200, 200, 0);
         ofRect(mBufferW * float(i)/mDisplayKeys, mBufferH, mBufferW/mDisplayKeys, 10);
      }
      ofPopStyle();
      
      for (int i=0; i<NUM_SEAOFGRAIN_VOICES; ++i)
         mVoices[i].Draw(mBufferW, mBufferH, sampleStart, sampleLength, mDisplayKeys);
      
      ofPopStyle();
      ofPopMatrix();
   }
}

int SeaOfGrain::GetSampleLength() const
{
   if (mSample == nullptr)
      return 0;
   return MIN(mSample->LengthInSamples(), gSampleRate * 10);
}

void SeaOfGrain::FilesDropped(vector<string> files, int x, int y)
{
   mLoading = true;
   
   mSample->Reset();
   
   mSample->Read(files[0].c_str());
   
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
}

void SeaOfGrain::ButtonClicked(ClickButton *button)
{
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

void SeaOfGrain::GetModuleDimensions(int& x, int& y)
{
   x = mBufferW + 10;
   y = mBufferY + mBufferH + 100;
}

void SeaOfGrain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SeaOfGrain::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void SeaOfGrain::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (voiceIdx == -1 || voiceIdx >= NUM_SEAOFGRAIN_VOICES)
      return;
   
   pitch -= mKeyboardBaseNote;
   if (velocity > 0)
      mVoices[voiceIdx].mADSR.Start(time, 1);
   else
      mVoices[voiceIdx].mADSR.Stop(time);
   mVoices[voiceIdx].mPitch = pitch;
   mVoices[voiceIdx].mPlay = 0;
   mVoices[voiceIdx].mPitchBend = modulation.pitchBend;
   mVoices[voiceIdx].mPressure = modulation.pressure;
   mVoices[voiceIdx].mModWheel = modulation.modWheel;
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
}


SeaOfGrain::GrainVoice::GrainVoice()
: mADSR(100,0,1,100)
, mPitch(0)
, mPitchBend(nullptr)
, mPressure(nullptr)
, mModWheel(nullptr)
, mGain(0)
{
   mGranulator.mGrainLengthMs = 150;
}

void SeaOfGrain::GrainVoice::Process(float* out, int outLength, float* sample, int sampleLength, int keyOffset, const vector<float>& slices)
{
   if (!mADSR.IsDone(gTime) && sampleLength > 0)
   {
      double time = gTime;
      for (int i=0; i<outLength; ++i)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(i) : 0;
         float pressure = mPressure ? mPressure->GetValue(i) : 0;
         float modwheel = mModWheel ? mModWheel->GetValue(i) : 0;
         if (pressure > 0)
         {
            mGranulator.mGrainSpacing = ofMap(pressure * pressure, 0, 1, .3f, 1.0f / MAX_GRAINS);
            mGranulator.mPosRandomizeMs = ofMap(pressure * pressure, 0, 1, 100, .03f);
         }
         mGranulator.mGrainLengthMs = ofMap(modwheel, -1, 1, 150-140, 150+140);
         
         double position = keyOffset + mPitch + pitchBend + MIN(.125f, mPlay);
         int intPosition = position;
         double positionPart = position - intPosition;
         
         float slice = ofMap(positionPart, 0, 1, slices[intPosition] * sampleLength, slices[intPosition+1] * sampleLength);
         
         float blend = .0005f;
         mGain = mGain * (1-blend) + pressure * blend;
         
         ChannelBuffer temp(sample, sampleLength);
         float outSample[1];
         outSample[0] = 0;
         mGranulator.Process(time, &temp, sampleLength, slice, outSample);
         outSample[0] *= sqrtf(mGain);
         outSample[0] *= mADSR.Value(time);
         out[i] += outSample[0];
         time += gInvSampleRateMs;
         mPlay += .001f;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }
}

void SeaOfGrain::GrainVoice::Draw(float w, float h, float offset, float length, int numPitches)
{
   if (!mADSR.IsDone(gTime))
   {
      if (mPitch < numPitches)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(0) : 0;
         float pressure = mPressure ? mPressure->GetValue(0) : 0;
         float modWheel = mModWheel ? mModWheel->GetValue(0) : 0;
         
         ofPushStyle();
         ofFill();
         float keyX = mPitch / numPitches * w;
         float keyXTop = keyX + pitchBend * w / numPitches;
         ofBeginShape();
         ofVertex(keyX, h);
         ofVertex(keyXTop, h - pressure * h);
         ofVertex(keyXTop +10, h - pressure * h);
         ofVertex(keyX+10, h);
         ofEndShape();
         ofPopStyle();
      }
      
      mGranulator.Draw(0, 0, w, h, offset, length, false);
      DrawText(ofToString(mGranulator.mPosRandomizeMs), 0, h+22);
      DrawText(ofToString(mGranulator.mGrainLengthMs), 0, h+42);
      DrawText(ofToString(mPitch), 0, h+62);
   }
}


