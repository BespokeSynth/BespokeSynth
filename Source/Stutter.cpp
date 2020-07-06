//
//  Stutter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/24/12.
//
//

#include "Stutter.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

bool Stutter::sQuantize = true;
int Stutter::sStutterSubdivide = 1;

Stutter::Stutter()
: mRecordBuffer(STUTTER_BUFFER_SIZE)
, mStutterBuffer(STUTTER_BUFFER_SIZE)
, mStuttering(false)
, mAutoStutter(false)
, mAutoCheckbox(nullptr)
, mFadeCheckbox(nullptr)
, mSubdivideSlider(nullptr)
, mNanopadScene(0)
, mCurrentStutter(kInterval_None, 0)
, mFadeStutter(false)
, mFreeStutterLength(.1f)
, mFreeStutterSpeed(1)
{
   TheTransport->AddListener(this, kInterval_16n, OffsetInfo(0, true), false);
   mBlendRamp.SetValue(0);
}

void Stutter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mAutoCheckbox = new Checkbox(this,"auto",4,25,&mAutoStutter);
   mFadeCheckbox = new Checkbox(this,"fade",4,4,&mFadeStutter);
   mSubdivideSlider = new IntSlider(this,"sub",45,25,50,15,&sStutterSubdivide,1,8);
}

Stutter::~Stutter()
{
   TheTransport->RemoveListener(this);
}

void Stutter::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(Stutter);
   
   float bufferSize = buffer->BufferSize();
   
   mRecordBuffer.SetNumChannels(buffer->NumActiveChannels());
   mStutterBuffer.SetNumActiveChannels(buffer->NumActiveChannels());

   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
      mRecordBuffer.WriteChunk(buffer->GetChannel(ch), bufferSize, ch);

   if (mBlendRamp.Target() > 0 || mBlendRamp.Value(time) > 0)
   {
      if (mCurrentStutter.interval == kInterval_None)
      {
         mStutterLengthRamp.Start(mFreeStutterLength * gSampleRate, 20);
         mStutterSpeed.Start(mFreeStutterSpeed, 20);
      }
      
      for (int i=0; i<bufferSize; ++i)
      {
         if (mCurrentStutter.interval != kInterval_None)
            mStutterLength = int(TheTransport->GetDuration(mCurrentStutter.interval) / 1000 * gSampleRate);
         else
            mStutterLength = int(mStutterLengthRamp.Value(time));
         
         float offset = mStutterPos;
         if (offset > mStutterLength)
            offset -= mStutterLength;
         int pos = int(offset);
         int posNext = int(offset+1) % mStutterLength;
         float a = offset - pos;

         for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         {
            float sample = GetStutterSampleWithWraparoundBlend(pos, ch);
            float nextSample = GetStutterSampleWithWraparoundBlend(posNext, ch);
            float stutterOut = (1-a)*sample + a*nextSample; //interpolate
            
            float fade = 1;
            if (mFadeStutter)
               fade -= (offset/mStutterLength) * (offset/mStutterLength);
            
            float blend = mBlendRamp.Value(time);
         
            buffer->GetChannel(ch)[i] = stutterOut * blend * fade + buffer->GetChannel(ch)[i] * (1-blend);
            buffer->GetChannel(ch)[i] = mJumpBlender[ch].Process(buffer->GetChannel(ch)[i], i);
         }
         
         //TODO(Ryan) what was this for?
         //if (blend == 0 && mBlendRamp.Target() == 0)
         //   break;

         mStutterPos += mStutterSpeed.Value(time);
         if (mStutterPos > mStutterLength)
            mStutterPos -= mStutterLength;
         if (mStutterPos < 0)
            mStutterPos += mStutterLength;

         time += gInvSampleRateMs;
      }
   }
}

float Stutter::GetStutterSampleWithWraparoundBlend(int pos, int ch)
{
   if (pos > mStutterLength - STUTTER_BLEND_WRAPAROUND_SAMPLES)
   {
      float a = float(mStutterLength - pos) / STUTTER_BLEND_WRAPAROUND_SAMPLES;
      int blendPos = pos - mStutterLength;
      pos = GetBufferReadPos(pos);
      blendPos = GetBufferReadPos(blendPos);
      pos = ofClamp(pos, 0, mCaptureLength);
      blendPos = ofClamp(blendPos, 0, mCaptureLength);
      return mStutterBuffer.GetChannel(ch)[pos]*a + mStutterBuffer.GetChannel(ch)[blendPos]*(1-a);
   }
   else
   {
      pos = GetBufferReadPos(pos);
      pos = ofClamp(pos, 0, mCaptureLength);
      return mStutterBuffer.GetChannel(ch)[pos];
   }
}

float Stutter::GetBufferReadPos(float stutterPos)
{
   return stutterPos;// + mCaptureLength - mStutterLength;
}

//TODO(Ryan) figure out how to blend out when we hit the rewrite button

void Stutter::StartStutter(StutterParams stutter)
{
   if (mAutoStutter || !mEnabled)
      return;
   
   mMutex.lock();
   mStutterStack.push_front(stutter);
   mMutex.unlock();
   
   bool quantize = sQuantize;
   if (stutter.interval == kInterval_None)
      quantize = false; //"free stutter" shouldn't be quantized
   
   if (!quantize)
      DoStutter(stutter);
}

void Stutter::EndStutter(StutterParams stutter)
{
   if (mAutoStutter || !mEnabled)
      return;
   
   mMutex.lock();
   bool hasNewStutter = false;
   if (mStutterStack.size() > 1 && *(mStutterStack.begin()) == stutter)
   {  //if we're removing the current stutter and there are held stutters
      mStutterStack.remove(stutter);
      stutter = *(mStutterStack.begin());   //use previously held as stutter
      hasNewStutter = true;
   }
   else
   {
      mStutterStack.remove(stutter);
   }
   mMutex.unlock();
   
   bool quantize = sQuantize;
   if (stutter.interval == kInterval_None && mStutterStack.empty())
      quantize = false; //"free stutter" shouldn't be quantized if we're releasing the only stutter
   
   if (!quantize)
   {
      if (mStutterStack.empty())
         StopStutter();
      else if (hasNewStutter)
         DoStutter(stutter);
   }
}

void Stutter::StopStutter()
{
   if (mStuttering)
   {
      mBlendRamp.Start(0, STUTTER_START_BLEND_MS);
      mStuttering = false;
   }
}

void Stutter::DoStutter(StutterParams stutter)
{
   if (mStuttering && stutter == mCurrentStutter)
      return;
   
   mCurrentStutter = stutter;
   
   if (!mStuttering ||
       int(TheTransport->GetDuration(mCurrentStutter.interval) / 1000 * gSampleRate) > mCaptureLength)
   {
      DoCapture();
   }
   else
   {
      //blend from the prior stutter to this new one
      for (int ch=0; ch<mStutterBuffer.NumActiveChannels(); ++ch)
      {
         float jumpBlend[JUMP_BLEND_SAMPLES];
         for (int i=0; i<JUMP_BLEND_SAMPLES; ++i)
            jumpBlend[i] = GetStutterSampleWithWraparoundBlend(int(mStutterPos)%mStutterLength+i, ch);
         mJumpBlender[ch].CaptureForJump(0, jumpBlend, JUMP_BLEND_SAMPLES, 0);
      }
   }
      
   mStutterPos = 0;
   mStuttering = true;
   mBlendRamp.Start(1, STUTTER_START_BLEND_MS);
   if (stutter.interval != kInterval_None)
      mStutterLength = int(TheTransport->GetDuration(stutter.interval) / 1000 * gSampleRate);
   else
      mStutterLength = mFreeStutterLength;
   if (stutter.speedBlendTime == 0)
      mStutterSpeed.SetValue(stutter.speedStart);
   else
      mStutterSpeed.Start(gTime,stutter.speedStart,stutter.speedEnd,gTime+stutter.speedBlendTime);
   mStutterLength /= sStutterSubdivide;
   mStutterLength = MAX(1,mStutterLength); //don't allow it to be zero
   mStutterLengthRamp.SetValue(mStutterLength);
}

void Stutter::DrawModule()
{
   if (!mEnabled)
      return;
   
   mAutoCheckbox->Draw();
   mFadeCheckbox->Draw();
   mSubdivideSlider->Draw();

   DrawStutterBuffer(4, 3, 90, 35);
}

void Stutter::DrawStutterBuffer(float x, float y, float width, float height)
{
   ofPushMatrix();
   ofTranslate(x,y);
   if (mStuttering)
      DrawAudioBuffer(width, height, &mStutterBuffer, 0, mCaptureLength, GetBufferReadPos(mStutterPos));
   ofPopMatrix();
}

void Stutter::GetModuleDimensions(float& width, float& height)
{
   if (mEnabled)
   {
      width = 98;
      height = 40;
   }
   else
   {
      width = 98;
      height = 0;
   }
}

float Stutter::GetEffectAmount()
{
   if ((mAutoStutter && mEnabled) || mStuttering)
      return 1;
   return 0;
}

void Stutter::DoCapture()
{
   mCaptureLength = int(TheTransport->GetDuration(mCurrentStutter.interval) / 1000 * gSampleRate);
   mCaptureLength = ofClamp(mCaptureLength, 0, STUTTER_BUFFER_SIZE-1);
   mCaptureLength /= sStutterSubdivide;
   for (int ch=0; ch<mStutterBuffer.NumActiveChannels(); ++ch)
      mRecordBuffer.ReadChunk(mStutterBuffer.GetChannel(ch), mCaptureLength, 0, ch);
}

void Stutter::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      UpdateEnabled();
   if (checkbox == mAutoCheckbox)
   {
      if (!mAutoStutter)
      {
         mMutex.lock();
         mStutterStack.clear();
         mMutex.unlock();
         StopStutter();
         TheTransport->UpdateListener(this, kInterval_16n);
      }
      else
      {
         TheTransport->UpdateListener(this, kInterval_8n);
      }
   }
}

void Stutter::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void Stutter::OnTimeEvent(double time)
{
   if (mEnabled)
   {
      if (mAutoStutter && TheTransport->GetMeasurePos(time) > .001f)  //don't auto-stutter downbeat
      {
         if (rand() % 4 == 0)
         {
            const StutterParams randomStutters[] = {StutterParams(kInterval_2n, 1),
                                                    StutterParams(kInterval_4n, 1),
                                                    StutterParams(kInterval_8n, 1),
                                                    StutterParams(kInterval_16n, 1),
                                                    StutterParams(kInterval_32n, 1),
                                                    StutterParams(kInterval_64n, 1),
                                                    StutterParams(kInterval_2n, -1),
                                                    StutterParams(kInterval_8n, .5f),
                                                    StutterParams(kInterval_8n, 2)};
            DoStutter(randomStutters[rand() % 9]);
         }
         else
         {
            StopStutter();
         }
      }
      
      if (sQuantize && !mAutoStutter)
      {
         mMutex.lock();
         if (mStutterStack.empty())
            StopStutter();
         else
            DoStutter(*mStutterStack.begin());
         mMutex.unlock();
      }
   }
}

void Stutter::UpdateEnabled()
{
   if (!mEnabled)
   {
      mMutex.lock();
      mStutterStack.clear();
      mMutex.unlock();
      StopStutter();
   }
}

void Stutter::LoadLayout(const ofxJSONElement& info)
{
}

void Stutter::SetUpFromSaveData()
{
}

void Stutter::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}

