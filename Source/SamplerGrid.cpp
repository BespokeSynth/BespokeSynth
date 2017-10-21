//
//  SamplerGrid.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/12/14.
//
//

#include "SamplerGrid.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Sample.h"
#include "Profiler.h"
#include "EnvOscillator.h"
#include "MidiController.h"

SamplerGrid::SamplerGrid()
: IAudioProcessor(gBufferSize)
, mPassthrough(true)
, mPassthroughCheckbox(nullptr)
, mRecordingSample(-1)
, mClearHeld(false)
, mVolume(1)
, mVolumeSlider(nullptr)
, mCols(0)
, mRows(0)
, mGridSamples(nullptr)
, mGridController(nullptr)
, mEditMode(false)
, mEditCheckbox(nullptr)
, mEditSampleX(100)
, mEditSampleY(5)
, mEditSampleWidth(395)
, mEditSampleHeight(200)
, mEditSample(nullptr)
, mEditStartSlider(nullptr)
, mEditEndSlider(nullptr)
, mDummyInt(0)
, mDuplicate(false)
, mDuplicateCheckbox(nullptr)
{
}

void SamplerGrid::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPassthroughCheckbox = new Checkbox(this,"passthrough",4,4,&mPassthrough);
   mVolumeSlider = new FloatSlider(this,"vol",4,20,90,15,&mVolume,0,2);
   mEditCheckbox = new Checkbox(this,"edit",4,36,&mEditMode);
   mDuplicateCheckbox = new Checkbox(this,"duplicate",4,56,&mDuplicate);
   mEditStartSlider = new IntSlider(this,"start",mEditSampleX,mEditSampleY+mEditSampleHeight,mEditSampleWidth,15,&mDummyInt,0,1);
   mEditEndSlider = new IntSlider(this,"end",mEditSampleX,mEditSampleY+mEditSampleHeight+14,mEditSampleWidth,15,&mDummyInt,0,1);
}

SamplerGrid::~SamplerGrid()
{
   delete[] mGridSamples;
}

void SamplerGrid::Init()
{
   IDrawableModule::Init();
   
   UpdateLights();
}

void SamplerGrid::Poll()
{
}

void SamplerGrid::Process(double time)
{
   Profiler profiler("SamplerGrid");
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   
   Clear(gWorkBuffer, gBufferSize);
   
   float volSq = mVolume * mVolume;
   
   for (int i=0; i<gBufferSize; ++i)
   {
      for (int j=0; j<mRows*mCols; ++j)
      {
         GridSample& sample = mGridSamples[j];
         float rampVal = sample.mRamp.Value(time);
         if (rampVal > 0 && sample.mPlayhead < sample.mSampleEnd)
         {
            gWorkBuffer[i] += sample.mSampleData[sample.mPlayhead] * rampVal * volSq;
            ++sample.mPlayhead;
            if (sample.mRamp.Target() == 1 &&
                sample.mPlayhead + SAMPLE_RAMP_MS/gInvSampleRateMs >= sample.mSampleEnd)
               sample.mRamp.Start(time, 0, time+SAMPLE_RAMP_MS);
         }
      }
      time += gInvSampleRateMs;
   }
   
   if (mRecordingSample != -1)
   {
      GridSample& sample = mGridSamples[mRecordingSample];
      for (int i=0; i<gBufferSize; ++i)
      {
         if (sample.mPlayhead < MAX_SAMPLER_GRID_LENGTH)
         {
            sample.mSampleData[sample.mPlayhead] = GetBuffer()->GetChannel(0)[i];// + gWorkBuffer[i];
            ++sample.mPlayhead;
            sample.mSampleLength = sample.mPlayhead;
            sample.mSampleStart = 0;
            sample.mSampleEnd = sample.mPlayhead;
         }
      }
   }
   
   if (mPassthrough)
   {
      for (int i=0; i<gBufferSize; ++i)
         gWorkBuffer[i] += GetBuffer()->GetChannel(0)[i];
   }
   
   GetVizBuffer()->WriteChunk(gWorkBuffer, bufferSize, 0);
   
   Add(GetTarget()->GetBuffer()->GetChannel(0), gWorkBuffer, bufferSize);
   
   GetBuffer()->Clear();
}

void SamplerGrid::ConnectGridController(IGridController* grid)
{
   mGridController = grid;
   
   delete[] mGridSamples;
   
   mCols = grid->NumCols();
   if (mLastColumnIsGroup)
      mCols -= 1;
   mRows = grid->NumRows()-1;
   
   mGridSamples = new GridSample[mRows*mCols];
   for (int i=0; i<mRows*mCols; ++i)
   {
      mGridSamples[i].mPlayhead = 0;
      mGridSamples[i].mHasSample = false;
      mGridSamples[i].mSampleLength = 0;
   }
   
   UpdateLights();
}

void SamplerGrid::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   bool bOn = velocity > 0;
   if (y < mRows && x < mCols)
   {
      int gridSampleIdx = GridToIdx(x, y);
      if (mDuplicate)
      {
         if (mEditSample)
         {
            mGridSamples[gridSampleIdx].mHasSample = true;
            mGridSamples[gridSampleIdx].mSampleLength = mEditSample->mSampleLength;
            mGridSamples[gridSampleIdx].mPlayhead = 0;
            mGridSamples[gridSampleIdx].mSampleStart = mEditSample->mSampleStart;
            mGridSamples[gridSampleIdx].mSampleEnd = mEditSample->mSampleEnd;
            BufferCopy(mGridSamples[gridSampleIdx].mSampleData, mEditSample->mSampleData, mEditSample->mSampleLength);
         }
         mDuplicate = false;
      }
      else if (mClearHeld && bOn)
      {
         mGridSamples[gridSampleIdx].mHasSample = false;
         mGridSamples[gridSampleIdx].mSampleLength = 0;
         mGridSamples[gridSampleIdx].mPlayhead = 0;
      }
      else if (mGridSamples[gridSampleIdx].mHasSample)
      {
         if (bOn)
            mGridSamples[gridSampleIdx].mPlayhead = mGridSamples[gridSampleIdx].mSampleStart;
         mGridSamples[gridSampleIdx].mRamp.Start(bOn ? 1 : 0, SAMPLE_RAMP_MS);
      }
      else if (bOn)
      {
         mGridSamples[gridSampleIdx].mHasSample = true;
         mRecordingSample = gridSampleIdx;
      }
      
      if (bOn)
         SetEditSample(&mGridSamples[gridSampleIdx]);
      
      if (!bOn && mRecordingSample == gridSampleIdx)
         mRecordingSample = -1;
   }
   else if (y == mRows)
   {
      mClearHeld = bOn;
   }
   else if (x == mCols)
   {
      for (int i=0; i<mCols; ++i)
      {
         int gridSampleIdx = GridToIdx(i, y);
         if (mClearHeld && bOn)
         {
            mGridSamples[gridSampleIdx].mHasSample = false;
            mGridSamples[gridSampleIdx].mSampleLength = 0;
            mGridSamples[gridSampleIdx].mPlayhead = 0;
         }
         else if (mGridSamples[gridSampleIdx].mHasSample && gridSampleIdx != mRecordingSample)
         {
            if (bOn)
               mGridSamples[gridSampleIdx].mPlayhead = mGridSamples[gridSampleIdx].mSampleStart;
            mGridSamples[gridSampleIdx].mRamp.Start(bOn ? 1 : 0, SAMPLE_RAMP_MS);
         }
      }
   }
   
   UpdateLights();
}

void SamplerGrid::SetEditSample(SamplerGrid::GridSample* sample)
{
   mEditSample = sample;
   mEditStartSlider->SetVar(&mEditSample->mSampleStart);
   mEditStartSlider->SetExtents(0, mEditSample->mSampleLength);
   mEditEndSlider->SetVar(&mEditSample->mSampleEnd);
   mEditEndSlider->SetExtents(0, mEditSample->mSampleLength);
}

void SamplerGrid::UpdateLights()
{
   if (!mGridController)
      return;
   
   //clear lights
   for (int x=0; x<mCols; ++x)
   {
      mGridController->SetLight(x,mRows,mClearHeld?kGridColor1Bright:kGridColorOff);
      for (int y=0; y<mRows; ++y)
      {
         int idx = GridToIdx(x, y);
         mGridController->SetLight(x,y,mGridSamples[idx].mHasSample?kGridColor2Bright:kGridColorOff);
      }
   }
   for (int y=0; y<mRows; ++y)
   {
      mGridController->SetLight(mCols,y,kGridColor1Bright);
   }
   mGridController->SetLight(mCols,mRows,mClearHeld?kGridColor1Bright:kGridColorOff);
}

void SamplerGrid::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void SamplerGrid::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mPassthroughCheckbox->Draw();
   mVolumeSlider->Draw();
   mEditCheckbox->Draw();
   
   if (mEditMode)
   {
      mDuplicateCheckbox->Draw();
      if (mEditSample && mEditSample->mSampleLength > 0)
      {
         ofPushMatrix();
         ofTranslate(mEditSampleX, mEditSampleY);
         DrawAudioBuffer(mEditSampleWidth, mEditSampleHeight, mEditSample->mSampleData, 0, mEditSample->mSampleLength, mEditSample->mPlayhead);
         ofPushStyle();
         ofFill();
         ofSetColor(0,0,0,80);
         float clipStartAmount = float(mEditSample->mSampleStart)/mEditSample->mSampleLength;
         float clipEndAmount = float(mEditSample->mSampleEnd)/mEditSample->mSampleLength;
         ofRect(0,0,mEditSampleWidth*clipStartAmount, mEditSampleHeight);
         ofRect(mEditSampleWidth*clipEndAmount, 0, mEditSampleWidth*(1-clipEndAmount),mEditSampleHeight);
         ofPopStyle();
         ofPopMatrix();
         mEditStartSlider->Draw();
         mEditEndSlider->Draw();
      }
   }
}

void SamplerGrid::GetModuleDimensions(int& width, int& height)
{
   if (mEditMode)
   {
      width = 500;
      height = 240;
   }
   else
   {
      width = 100;
      height = 54;
   }
}

void SamplerGrid::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   if (x >= mEditSampleX && x < mEditSampleX + mEditSampleWidth &&
       y >= mEditSampleY && y < mEditSampleY + mEditSampleHeight)
   {
      if (mEditSample)
         TheSynth->GrabSample(mEditSample->mSampleData + mEditSample->mSampleStart,
                              mEditSample->mSampleEnd - mEditSample->mSampleStart,
                              K(window));
   }
}

void SamplerGrid::FilesDropped(vector<string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x,y,&sample);
}

void SamplerGrid::SampleDropped(int x, int y, Sample* sample)
{
   assert(sample);
   int numSamples = sample->LengthInSamples();
   
   if (numSamples <= 0)
      return;
   
   if (mEditSample == nullptr)
      return;
   
   mEditSample->mPlayhead = 0;
   mEditSample->mHasSample = true;
   mEditSample->mSampleLength = MIN(MAX_SAMPLER_GRID_LENGTH, numSamples);
   mEditSample->mSampleStart = 0;
   mEditSample->mSampleEnd = mEditSample->mSampleLength;
   
   for (int i=0; i<mEditSample->mSampleLength; ++i)
      mEditSample->mSampleData[i] = sample->Data()[i];
   
   SetEditSample(mEditSample); //refresh
}

void SamplerGrid::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("last_column_is_group", moduleInfo, true);
   
   SetUpFromSaveData();
}

void SamplerGrid::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mLastColumnIsGroup = mModuleSaveData.GetBool("last_column_is_group");
}

void SamplerGrid::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void SamplerGrid::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SamplerGrid::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   
}

void SamplerGrid::CheckboxUpdated(Checkbox* checkbox)
{
}

namespace
{
   const int kSaveStateRev = 0;
}

void SamplerGrid::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   for (int i=0; i<mCols*mRows; ++i)
   {
      out << mGridSamples[i].mPlayhead;
      out << mGridSamples[i].mHasSample;
      out << mGridSamples[i].mSampleLength;
      out << mGridSamples[i].mSampleStart;
      out << mGridSamples[i].mSampleEnd;
      if (mGridSamples[i].mHasSample)
         out.Write(mGridSamples[i].mSampleData, mGridSamples[i].mSampleLength);
   }
}

void SamplerGrid::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   //LoadStateValidate(false); //TODO(Ryan) temp hack fix because samplergrid was loading funny
   
   for (int i=0; i<mCols*mRows; ++i)
   {
      in >> mGridSamples[i].mPlayhead;
      in >> mGridSamples[i].mHasSample;
      in >> mGridSamples[i].mSampleLength;
      in >> mGridSamples[i].mSampleStart;
      in >> mGridSamples[i].mSampleEnd;
      if (mGridSamples[i].mHasSample)
         in.Read(mGridSamples[i].mSampleData, mGridSamples[i].mSampleLength);
   }
}

