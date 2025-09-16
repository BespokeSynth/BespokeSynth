/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
{
}

void SamplerGrid::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid(this, "uigrid", 2, 2, 90, 90, mCols, mRows);
   mGrid->SetListener(this);
   mGrid->SetMomentary(true);
   mPassthroughCheckbox = new Checkbox(this, "passthrough", mGrid, kAnchor_Right, &mPassthrough);
   mVolumeSlider = new FloatSlider(this, "vol", mPassthroughCheckbox, kAnchor_Below, 90, 15, &mVolume, 0, 2);
   mClearCheckbox = new Checkbox(this, "clear", mVolumeSlider, kAnchor_Below, &mClear);
   mEditCheckbox = new Checkbox(this, "edit", mClearCheckbox, kAnchor_Below, &mEditMode);
   mDuplicateCheckbox = new Checkbox(this, "duplicate", mEditCheckbox, kAnchor_Below, &mDuplicate);
   mEditStartSlider = new IntSlider(this, "start", mEditSampleX, mEditSampleY + mEditSampleHeight + 1, mEditSampleWidth, 15, &mDummyInt, 0, 1);
   mEditEndSlider = new IntSlider(this, "end", mEditStartSlider, kAnchor_Below, mEditSampleWidth, 15, &mDummyInt, 0, 1);
   mGridControlTarget = new GridControlTarget(this, "grid", 4, 4);
   mGridControlTarget->PositionTo(mClearCheckbox, kAnchor_Right);

   InitGrid();
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
   PROFILER(SamplerGrid);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();

      return;
   }

   ComputeSliders(0);

   int bufferSize = GetBuffer()->BufferSize();

   Clear(gWorkBuffer, gBufferSize);

   float volSq = mVolume * mVolume;

   for (int i = 0; i < gBufferSize; ++i)
   {
      for (int j = 0; j < mRows * mCols; ++j)
      {
         GridSample& sample = mGridSamples[j];
         float rampVal = sample.mRamp.Value(time);
         if (rampVal > 0 && sample.mPlayhead < sample.mSampleEnd)
         {
            gWorkBuffer[i] += sample.mSampleData[sample.mPlayhead] * rampVal * volSq;
            ++sample.mPlayhead;
            if (sample.mRamp.Target(time) == 1 &&
                sample.mPlayhead + SAMPLE_RAMP_MS / gInvSampleRateMs >= sample.mSampleEnd)
               sample.mRamp.Start(time, 0, time + SAMPLE_RAMP_MS);
         }
      }
      time += gInvSampleRateMs;
   }

   if (mRecordingSample != -1)
   {
      GridSample& sample = mGridSamples[mRecordingSample];
      for (int i = 0; i < gBufferSize; ++i)
      {
         if (GetBuffer()->GetChannel(0)[i] != 0)
            sample.mHasSample = true;
         if (sample.mPlayhead < MAX_SAMPLER_GRID_LENGTH && sample.mHasSample)
         {
            sample.mSampleData[sample.mPlayhead] = GetBuffer()->GetChannel(0)[i]; // + gWorkBuffer[i];
            ++sample.mPlayhead;
            sample.mSampleLength = sample.mPlayhead;
            sample.mSampleStart = 0;
            sample.mSampleEnd = sample.mPlayhead;
         }
      }
   }

   if (mPassthrough)
   {
      for (int i = 0; i < gBufferSize; ++i)
         gWorkBuffer[i] += GetBuffer()->GetChannel(0)[i];
   }

   GetVizBuffer()->WriteChunk(gWorkBuffer, bufferSize, 0);

   Add(target->GetBuffer()->GetChannel(0), gWorkBuffer, bufferSize);

   GetBuffer()->Reset();
}

void SamplerGrid::OnControllerPageSelected()
{
   if (mGridControlTarget->GetGridController())
      mGridControlTarget->GetGridController()->ResetLights();

   /*delete[] mGridSamples;
   
   mCols = grid->NumCols();
   if (mLastColumnIsGroup)
      mCols -= 1;
   mRows = grid->NumRows();
   
   InitGrid();*/

   UpdateLights();
}

void SamplerGrid::InitGrid()
{
   delete[] mGridSamples;
   mGridSamples = new GridSample[mRows * mCols];
   for (int i = 0; i < mRows * mCols; ++i)
   {
      mGridSamples[i].mPlayhead = 0;
      mGridSamples[i].mHasSample = false;
      mGridSamples[i].mSampleLength = 0;
   }

   mGrid->SetGrid(mCols, mRows);
}

void SamplerGrid::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   OnGridButton(col, row, value, nullptr);
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
      else if (mClear && bOn)
      {
         mGridSamples[gridSampleIdx].mHasSample = false;
         mGridSamples[gridSampleIdx].mSampleLength = 0;
         mGridSamples[gridSampleIdx].mPlayhead = 0;
      }
      else if (mGridSamples[gridSampleIdx].mHasSample)
      {
         if (bOn)
            mGridSamples[gridSampleIdx].mPlayhead = mGridSamples[gridSampleIdx].mSampleStart;
         mGridSamples[gridSampleIdx].mRamp.Start(gTime, bOn ? 1 : 0, gTime + SAMPLE_RAMP_MS);
      }
      else if (bOn)
      {
         if (mRecordingSample != -1)
            mGridSamples[mRecordingSample].mRecordingArmed = false;
         mGridSamples[gridSampleIdx].mRecordingArmed = true;
         mRecordingSample = gridSampleIdx;
      }

      if (bOn)
         SetEditSample(&mGridSamples[gridSampleIdx]);

      if (!bOn && mRecordingSample == gridSampleIdx)
         mRecordingSample = -1;
   }
   /*else if (x == mCols)
   {
      for (int i=0; i<mCols; ++i)
      {
         int gridSampleIdx = GridToIdx(i, y);
         if (mClear && bOn)
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
   }*/

   UpdateLights();
}

void SamplerGrid::PlayNote(NoteMessage note)
{
   OnGridButton(note.pitch % mCols, (note.pitch / mCols) % mRows, note.velocity / 127.0f, nullptr);
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
   if (!mGridControlTarget->GetGridController())
      return;

   //clear lights
   for (int x = 0; x < mCols; ++x)
   {
      mGridControlTarget->GetGridController()->SetLight(x, mRows, mClear ? kGridColor1Bright : kGridColorOff);
      for (int y = 0; y < mRows; ++y)
      {
         int idx = GridToIdx(x, y);
         mGridControlTarget->GetGridController()->SetLight(x, y, mGridSamples[idx].mHasSample ? kGridColor2Bright : kGridColorOff);
      }
   }
   for (int y = 0; y < mRows; ++y)
   {
      mGridControlTarget->GetGridController()->SetLight(mCols, y, kGridColor1Bright);
   }
   mGridControlTarget->GetGridController()->SetLight(mCols, mRows, mClear ? kGridColor1Bright : kGridColorOff);
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
   mClearCheckbox->Draw();
   mGrid->Draw();
   mGridControlTarget->Draw();

   ofPushStyle();
   ofSetColor(255, 255, 255, 100);
   ofFill();
   for (int x = 0; x < mCols; ++x)
   {
      for (int y = 0; y < mRows; ++y)
      {
         int idx = GridToIdx(x, y);
         if (mGridSamples[idx].mHasSample)
         {
            ofVec2f cellPos = mGrid->GetCellPosition(x, y);
            ofVec2f gridSize = mGrid->IClickable::GetDimensions();
            ofVec2f gridPos = mGrid->IClickable::GetPosition();
            ofRect(gridPos.x + cellPos.x, gridPos.y + cellPos.y, gridSize.x / mCols, gridSize.y / mCols);
         }
      }
   }
   ofPopStyle();

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
         ofSetColor(0, 0, 0, 80);
         float clipStartAmount = float(mEditSample->mSampleStart) / mEditSample->mSampleLength;
         float clipEndAmount = float(mEditSample->mSampleEnd) / mEditSample->mSampleLength;
         ofRect(0, 0, mEditSampleWidth * clipStartAmount, mEditSampleHeight);
         ofRect(mEditSampleWidth * clipEndAmount, 0, mEditSampleWidth * (1 - clipEndAmount), mEditSampleHeight);
         ofPopStyle();
         ofPopMatrix();
         mEditStartSlider->Draw();
         mEditEndSlider->Draw();
      }
   }
}

void SamplerGrid::GetModuleDimensions(float& width, float& height)
{
   if (mEditMode)
   {
      width = mEditSampleWidth;
      height = mEditSampleY + mEditSampleHeight + 17 * 2;
   }
   else
   {
      width = 188;
      height = mEditSampleY;
   }
}

void SamplerGrid::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mGrid->TestClick(x, y, right);

   if (x >= mEditSampleX && x < mEditSampleX + mEditSampleWidth &&
       y >= mEditSampleY && y < mEditSampleY + mEditSampleHeight)
   {
      if (mEditSample)
      {
         //TODO(Ryan) multichannel
         ChannelBuffer temp(mEditSample->mSampleData + mEditSample->mSampleStart, mEditSample->mSampleEnd - mEditSample->mSampleStart);
         TheSynth->GrabSample(&temp, "gridsample", K(window));
      }
   }
}

void SamplerGrid::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mGrid->MouseReleased();
}

void SamplerGrid::FilesDropped(std::vector<std::string> files, int x, int y)
{
   Sample sample;
   sample.Read(files[0].c_str());
   SampleDropped(x, y, &sample);
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

   //TODO(Ryan) multichannel
   for (int i = 0; i < mEditSample->mSampleLength; ++i)
      mEditSample->mSampleData[i] = sample->Data()->GetChannel(0)[i];

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

void SamplerGrid::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void SamplerGrid::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void SamplerGrid::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void SamplerGrid::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void SamplerGrid::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   for (int i = 0; i < mCols * mRows; ++i)
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

void SamplerGrid::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   //LoadStateValidate(false); //TODO(Ryan) temp hack fix because samplergrid was loading funny

   for (int i = 0; i < mCols * mRows; ++i)
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
