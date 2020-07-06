//
//  SlowLayers.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/13/15.
//
//

#include "SlowLayers.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"

SlowLayers::SlowLayers()
: IAudioProcessor(gBufferSize)
, mBuffer(nullptr)
, mLoopPos(0)
, mNumBars(1)
, mVol(1)
, mSmoothedVol(1)
, mVolSlider(nullptr)
, mNumBarsSelector(nullptr)
, mFeedInSlider(nullptr)
, mFeedIn(1)
{
   //TODO(Ryan) buffer sizes
   mBuffer = new float[MAX_BUFFER_SIZE];
   Clear();
}

void SlowLayers::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mClearButton = new ClickButton(this,"clear", 147, 102);
   mVolSlider = new FloatSlider(this,"volume", 4, 102, 110, 15, &mVol, 0, 2);
   mNumBarsSelector = new DropdownList(this,"num bars",165, 4, &mNumBars);
   mFeedInSlider = new FloatSlider(this,"feed in", 4, 120, 110, 15, &mFeedIn, 0, 1);
   
   mNumBarsSelector->AddLabel(" 1 ",1);
   mNumBarsSelector->AddLabel(" 2 ",2);
   mNumBarsSelector->AddLabel(" 3 ",3);
   mNumBarsSelector->AddLabel(" 4 ",4);
   mNumBarsSelector->AddLabel(" 6 ",6);
   mNumBarsSelector->AddLabel(" 8 ",8);
   mNumBarsSelector->AddLabel("12 ",12);
}

SlowLayers::~SlowLayers()
{
   delete[] mBuffer;
}

void SlowLayers::Process(double time)
{
   PROFILER(SlowLayers);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   
   int loopLengthInSamples = LoopLength();
   
   int layers = 4;
   for (int i=0; i<bufferSize; ++i)
   {
      float smooth = .001f;
      mSmoothedVol = mSmoothedVol * (1-smooth) + mVol * smooth;
      float volSq = mSmoothedVol * mSmoothedVol;
      
      float measurePos = TheTransport->GetMeasureTime(time);
      FloatWrap(measurePos, 1 << layers * mNumBars);
      int offset = measurePos * loopLengthInSamples;
      
      mBuffer[offset % loopLengthInSamples] += GetBuffer()->GetChannel(0)[i] * mFeedIn;
      
      float output = (1-mFeedIn)*GetBuffer()->GetChannel(0)[i];
      for (int i=0; i<layers; ++i)
         output += GetInterpolatedSample(offset/float(1<<i), mBuffer, loopLengthInSamples);
      
      output *= volSq;
      
      out[i] += output;
      
      time += gInvSampleRateMs;
   }
   
   Add(out, GetBuffer()->GetChannel(0), bufferSize);
   
   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);
   
   GetBuffer()->Reset();
}

int SlowLayers::LoopLength() const
{
   return TheTransport->GetDuration(kInterval_1n) * mNumBars * gSampleRate / 1000;
}

void SlowLayers::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofPushMatrix();
   
   ofTranslate(BUFFER_X,BUFFER_Y);
   
   DrawAudioBuffer(BUFFER_W, BUFFER_H, mBuffer, 0, LoopLength(), TheTransport->GetMeasurePos(gTime)*LoopLength(), mVol);
   ofSetColor(255,255,0,gModuleDrawAlpha);
   for (int i=1; i<mNumBars; ++i)
   {
      float x = BUFFER_W/mNumBars * i;
      ofLine(x,BUFFER_H/2-5,x,BUFFER_H/2+5);
   }
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   ofPopMatrix();
   
   mClearButton->Draw();
   mNumBarsSelector->Draw();
   mVolSlider->Draw();
   mFeedInSlider->Draw();
}

void SlowLayers::Clear()
{
   ::Clear(mBuffer, MAX_BUFFER_SIZE);
}

void SlowLayers::SetNumBars(int numBars)
{
   mNumBars = numBars;
}

void SlowLayers::GetModuleDimensions(float& width, float& height)
{
   width = 197;
   height = 155;
}

void SlowLayers::ButtonClicked(ClickButton* button)
{
   if (button == mClearButton)
      ::Clear(mBuffer, MAX_BUFFER_SIZE);
}

void SlowLayers::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SlowLayers::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void SlowLayers::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void SlowLayers::CheckboxUpdated(Checkbox* checkbox)
{
}

void SlowLayers::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SlowLayers::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

