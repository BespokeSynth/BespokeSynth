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
//  DrumPlayer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "DrumPlayer.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "ModularSynth.h"
#include "MidiController.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "UIControlMacros.h"
#include "SamplePlayer.h"

using namespace juce;

DrumPlayer::DrumPlayer()
: mSpeed(1)
, mSpeedRandomization(0)
, mVolume(1)
, mLoadedKit(0)
, mVolSlider(nullptr)
, mSpeedSlider(nullptr)
, mKitSelector(nullptr)
, mEditMode(false)
, mEditCheckbox(nullptr)
, mSaveButton(nullptr)
, mNewKitButton(nullptr)
, mAuditionSampleIdx(0)
, mAuditionInc(0)
, mAuditionSlider(nullptr)
, mNewKitNameEntry(nullptr)
, mLoadingSamples(false)
, mShuffleButton(nullptr)
, mSelectedHitIdx(0)
, mOutputBuffer(gBufferSize)
, mMonoOutput(false)
, mMonoCheckbox(nullptr)
, mGridControlTarget(nullptr)
, mNoteInputBuffer(this)
, mNeedSetup(true)
, mNoteRepeat(false)
, mQuantizeInterval(kInterval_None)
{

   ReadKits();
   
   strcpy(mNewKitName, "new");
}

void DrumPlayer::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, mQuantizeInterval, OffsetInfo(0, true), false);
}

DrumPlayer::~DrumPlayer()
{
   TheTransport->RemoveListener(this);

   for (int i = 0; i < mIndividualOutputs.size(); ++i)
      delete mIndividualOutputs[i];
}

void DrumPlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",4,2,100,15,&mVolume,0,2);
   mSpeedSlider = new FloatSlider(this,"speed",4,18,100,15,&mSpeed,0.2f,3);
   mSpeedRandomizationSlider = new FloatSlider(this, "speed rnd", 4, 34, 100, 15, &mSpeedRandomization, 0, .2f);
   mKitSelector = new DropdownList(this,"kit",4,50,&mLoadedKit);
   mEditCheckbox = new Checkbox(this,"edit",73,52,&mEditMode);
   //mSaveButton = new ClickButton(this,"save current",200,22);
   //mNewKitButton = new ClickButton(this,"new kit", 200, 4);
   //mNewKitNameEntry = new TextEntry(this,"kitname",200, 40,7,mNewKitName);
   mAuditionSlider = new FloatSlider(this,"aud",140,50,40,15,&mAuditionInc,-1,1,0);
   mMonoCheckbox = new Checkbox(this,"mono",mVolSlider,kAnchor_Right_Padded,&mMonoOutput);
   mShuffleButton = new ClickButton(this,"shuffle",140,34);
   mGridControlTarget = new GridControlTarget(this, "grid", 4, 50);
   mQuantizeIntervalSelector = new DropdownList(this, "quantize", 200, 4, (int*)(&mQuantizeInterval));
   mNoteRepeatCheckbox = new Checkbox(this, "repeat", 200, 22, &mNoteRepeat);

   mKitSelector->SetShowing(false); //TODO(Ryan) replace "kits" concept with a better form of serialization

   mQuantizeIntervalSelector->AddLabel("none", kInterval_None);
   mQuantizeIntervalSelector->AddLabel("4n", kInterval_4n);
   mQuantizeIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mQuantizeIntervalSelector->AddLabel("8n", kInterval_8n);
   mQuantizeIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mQuantizeIntervalSelector->AddLabel("16n", kInterval_16n);
   mQuantizeIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mQuantizeIntervalSelector->AddLabel("32n", kInterval_32n);
   mQuantizeIntervalSelector->AddLabel("64n", kInterval_64n);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mDrumHits[i].CreateUIControls(this, i);
   
   for (int i=0; i<mKits.size(); ++i)
      mKitSelector->AddLabel(mKits[i].mName.c_str(), i);
   
   UpdateVisibleControls();
   
   GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kBottom);
   mSpeedRandomizationSlider->SetMode(FloatSlider::kSquare);
}

void DrumPlayer::DrumHit::CreateUIControls(DrumPlayer* owner, int index)
{
   UIBLOCK(310,37);
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER owner //change owner
   FLOATSLIDER_DIGITS(mVolSlider, ("vol "+ofToString(index)).c_str(),&mVol,0,2,2);
   FLOATSLIDER_DIGITS(mSpeedSlider, ("speed "+ofToString(index)).c_str(),&mSpeed,.2f,3,2);
   FLOATSLIDER(mPanSlider, ("pan "+ofToString(index)).c_str(),&mPan,-1,1);
   INTSLIDER(mWidenSlider, ("widen "+ofToString(index)).c_str(),&mWiden,-150,150);
   FLOATSLIDER(mStartOffsetSlider, ("start "+ofToString(index)).c_str(),&mStartOffset,0,1);
   CHECKBOX(mIndividualOutputCheckbox, ("single out "+ofToString(index)).c_str(),&mHasIndividualOutput);
   INTSLIDER(mLinkIdSlider, ("linkid " + ofToString(index)).c_str(), &mLinkId, -1, 5);
   CHECKBOX(mUseEnvelopeCheckbox, ("envelope "+ofToString(index)).c_str(),&mUseEnvelope);
   FLOATSLIDER(mEnvelopeLengthSlider, ("view ms "+ofToString(index)).c_str(),&mEnvelopeLength,10,2000);
   DROPDOWN(mHitCategoryDropdown,("hitcategory"+ofToString(index)).c_str(),&mHitCategoryIndex,100);
   UICONTROL_CUSTOM(mEnvelopeDisplay, new ADSRDisplay(UICONTROL_BASICS(("envelopedisplay "+ofToString(index)).c_str()),135, 100,&mEnvelope));
   BUTTON_STYLE(mGrabSampleButton, ("grab " + ofToString(index)).c_str(), ButtonDisplayStyle::kGrabSample);
   ENDUIBLOCK0();
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER this //reset
   
   int x = 5 + (index % 4) * 70;
   int y = 70 + (3-(index / 4)) * 70;
   mTestButton = new ClickButton(owner,("test "+ofToString(index)).c_str(),x+5,y+38,ButtonDisplayStyle::kPlay);
   mRandomButton = new ClickButton(owner,("random "+ofToString(index)).c_str(),x+5,y+53);
   
   UpdateHitDirectoryDropdown();
   
   mOwner = owner;
}

namespace
{
   static std::list<std::string> sHitDirectories;
}

//static
void DrumPlayer::SetUpHitDirectories()
{
   sHitDirectories.clear();
   File parentDirectory(ofToDataPath("drums"));
   Array<File> hitDirs;
   parentDirectory.findChildFiles(hitDirs, File::findDirectories, true);
   for (auto dir : hitDirs)
   {
      Array<File> filesInDir;
      dir.findChildFiles(filesInDir, File::findFiles, false);
      if (filesInDir.size() > 0)
         sHitDirectories.push_back(dir.getRelativePathFrom(parentDirectory).replaceCharacter('\\', '/').toStdString());
   }
}

void DrumPlayer::DrumHit::UpdateHitDirectoryDropdown()
{
   mHitCategoryDropdown->Clear();
   for (auto dir : sHitDirectories)
      mHitCategoryDropdown->AddLabel(dir, mHitCategoryDropdown->GetNumValues());
   mHitCategoryIndex = -1;
   for (int i=0; i<mHitCategoryDropdown->GetNumValues(); ++i)
   {
      if (mHitCategory == mHitCategoryDropdown->GetLabel(i))
         mHitCategoryIndex = i;
   }
}

void DrumPlayer::Poll()
{
   if (mNeedSetup)
      SetUpNewDrumPlayer();

   UpdateLights();
}

void DrumPlayer::SetUpNewDrumPlayer()
{
   ofxJSONElement root;
   root.open(ofToDataPath("drums/drums.json"));

   std::array<std::string, NUM_DRUM_HITS> categories = { "808kit/Kick", "808kit/Snare", "808kit/HatClosed", "808kit/HatOpen", "808kit/Kick", "808kit/Clap", "808kit/HatClosed", "808kit/Perc",
                                               "808kit/Kick", "808kit/Snare", "808kit/HatClosed", "808kit/HatOpen", "808kit/Kick", "808kit/Clap", "808kit/HatClosed", "808kit/Perc" };
   for (auto i = 0; i < root["directories"].size() && i < categories.size(); ++i)
      categories[i] = root["directories"][i].asString();

   for (int i = 0; i < NUM_DRUM_HITS; ++i)
   {
      std::string category = categories[i % categories.size()];
      File dir(ofToDataPath("drums/" + category));
      if (dir.exists())
      {
         mDrumHits[i].mHitCategory = category;
         mDrumHits[i].UpdateHitDirectoryDropdown();
      }
      mDrumHits[i].LoadRandomSample();

      if (i == 2 || i == 3)
         mDrumHits[i].mLinkId = 0;
   }

   mNeedSetup = false;
}

void DrumPlayer::UpdateVisibleControls()
{
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mDrumHits[i].SetUIControlsShowing(i == mSelectedHitIdx && mEditMode);
}

void DrumPlayer::DrumHit::SetUIControlsShowing(bool showing)
{
   mVolSlider->SetShowing(showing);
   mSpeedSlider->SetShowing(showing);
   mUseEnvelopeCheckbox->SetShowing(showing);
   mEnvelopeDisplay->SetShowing(showing);
   mPanSlider->SetShowing(showing && mOwner->mMonoOutput == false);
   mWidenSlider->SetShowing(showing && mOwner->mMonoOutput == false);
   mStartOffsetSlider->SetShowing(showing);
   mIndividualOutputCheckbox->SetShowing(showing);
   mLinkIdSlider->SetShowing(showing);
   mEnvelopeLengthSlider->SetShowing(showing);
   mHitCategoryDropdown->SetShowing(showing);
   mGrabSampleButton->SetShowing(showing);
}

void DrumPlayer::LoadKit(int kit)
{
   if (kit >= 0 && kit < mKits.size())
   {
      // Maschine samples are in /Users/Shared/Maschine Library/Samples
      mLoadedKit = kit;

      LoadSampleLock();
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         mDrumHits[i].mSample.Read(mKits[kit].mSampleFiles[i].c_str());
         mDrumHits[i].mLinkId = mKits[kit].mLinkIds[i];
         mDrumHits[i].mVol = mKits[kit].mVols[i];
         mDrumHits[i].mSpeed = mKits[kit].mSpeeds[i];
         mDrumHits[i].mPan = mKits[kit].mPans[i];
         mDrumHits[i].mEnvelopeLength = mDrumHits[i].mSample.LengthInSamples() * gInvSampleRateMs;
      }
      LoadSampleUnlock();
   }
}

void DrumPlayer::LoadSampleLock()
{
   mLoadSamplesAudioMutex.lock();
   mLoadSamplesDrawMutex.lock();
   mLoadingSamples = true;
}

void DrumPlayer::LoadSampleUnlock()
{
   mLoadingSamples = false;
   mLoadSamplesDrawMutex.unlock();
   mLoadSamplesAudioMutex.unlock();
}

void DrumPlayer::Process(double time)
{
   PROFILER(DrumPlayer);
   
   if (!mEnabled)
      return;
   
   mNoteInputBuffer.Process(time);
   
   int numChannels = mMonoOutput ? 1 : 2;
   
   ComputeSliders(0);
   SyncOutputBuffer(numChannels);
   for (auto output : mIndividualOutputs)
      output->mVizBuffer->SetNumChannels(numChannels);
   mOutputBuffer.SetNumActiveChannels(numChannels);
   
   int bufferSize = gBufferSize;
   
   float volSq = mVolume * mVolume * .5f;
   
   mOutputBuffer.Clear();
   
   if (!mLoadingSamples)
   {
      mLoadSamplesAudioMutex.lock();
      mLoadingSamples = true;
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         int individualOutputIndex = GetIndividualOutputIndex(i);
         gWorkChannelBuffer.SetNumActiveChannels(numChannels);
         if (mDrumHits[i].Process(time, mSpeed, volSq, &gWorkChannelBuffer, bufferSize))
         {
            for (int ch=0; ch<numChannels; ++ch)
            {
               if (individualOutputIndex != -1)
               {
                  int targetIndex = individualOutputIndex + 1;
                  IAudioReceiver* targetOut = GetTarget(targetIndex);
                  if (targetOut)
                     Add(targetOut->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
                  mIndividualOutputs[individualOutputIndex]->mVizBuffer->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
               }
               else
               {
                  Add(mOutputBuffer.GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
               }
            }
         }
         else
         {
            if (individualOutputIndex != -1)
            {
               for (int ch=0; ch<numChannels; ++ch)
                  mIndividualOutputs[individualOutputIndex]->mVizBuffer->WriteChunk(gZeroBuffer, bufferSize, ch);
            }
         }
      }
      mLoadingSamples = false;
      mLoadSamplesAudioMutex.unlock();
   }
   
   IAudioReceiver* target = GetTarget();
   for (int ch=0; ch<numChannels; ++ch)
   {
      GetVizBuffer()->WriteChunk(mOutputBuffer.GetChannel(ch), bufferSize, ch);
   
      if (target)
         Add(target->GetBuffer()->GetChannel(ch), mOutputBuffer.GetChannel(ch), bufferSize);
   }
}

void DrumPlayer::DrumHit::StartPlayhead(double time, float startOffsetPercent, float velocity)
{
   mCurrentPlayheadIndex = (mCurrentPlayheadIndex + 1) % mPlayheads.size();
   for (size_t i = 0; i < mPlayheads.size(); ++i)
   {
      if (i == mCurrentPlayheadIndex)
      {
         mPlayheads[i].mStartTime = time;
         mPlayheads[i].mCutOffTime = -1;
         mPlayheads[i].mOffset = startOffsetPercent * mSample.LengthInSamples();
         mPlayheads[i].mEnvelopeTime = 0;
         mPlayheads[i].mEnvelopeScale = ofLerp(.2f, 1, velocity);
         mPlayheads[i].mSpeedTweak = ofRandom(1-mOwner->mSpeedRandomization, 1+ mOwner->mSpeedRandomization);
      }
      else
      {
         mPlayheads[i].mCutOffTime = time;
      }
   }
}

void DrumPlayer::DrumHit::StopLinked(double time)
{
   for (size_t i = 0; i < mPlayheads.size(); ++i)
   {
      if (mPlayheads[i].mCutOffTime == -1)
         mPlayheads[i].mCutOffTime = time;
   }
}

float DrumPlayer::DrumHit::GetPlayProgress(double time)
{
   int playheadIdx = -1;
   double startTime = -1;
   for (int i = 0; i < (int)mPlayheads.size(); ++i)
   {
      if (mPlayheads[i].mStartTime <= time && mPlayheads[i].mStartTime > startTime)
      {
         startTime = mPlayheads[i].mStartTime;
         playheadIdx = i;
      }
   }

   if (startTime != -1 && mSample.Data() != nullptr)
      return mPlayheads[playheadIdx].mOffset / mSample.LengthInSamples();
   return 1;
}

bool DrumPlayer::DrumHit::Process(double time, float speed, float vol, ChannelBuffer* out, int bufferSize)
{
   ChannelBuffer* sampleData = mSample.Data();
   speed *= mSpeed;

   for (int i = 0; i < bufferSize; ++i)
   {
      float sampleSpeed = speed;
      if (mPitchBend != nullptr)
         sampleSpeed *= ofMap(mPitchBend->GetValue(i), -.5f, .5f, 0, 2);

      for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
         gWorkBuffer[ch] = 0;

      for (size_t playhead = 0; playhead < mPlayheads.size(); ++playhead)
      {
         if (mPlayheads[playhead].mStartTime != -1 && time > mPlayheads[playhead].mStartTime && mPlayheads[playhead].mOffset < mSample.LengthInSamples())
         {
            for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
            {
               int dataChannel = MIN(ch, sampleData->NumActiveChannels() - 1);
               float sample = GetInterpolatedSample(mPlayheads[playhead].mOffset, sampleData->GetChannel(dataChannel), mSample.LengthInSamples());
               sample *= mVelocity * vol * mVol * mVol;
               if (mUseEnvelope)
                  sample *= mEnvelope.Value(mPlayheads[playhead].mEnvelopeTime);

               if (mPlayheads[playhead].mCutOffTime != -1 && time > mPlayheads[playhead].mCutOffTime)
               {
                  float fade = ofMap(time - mPlayheads[playhead].mCutOffTime, 0, .25f, 1, 0, K(clamp));
                  sample *= fade;
                  if (fade == 0)
                     mPlayheads[playhead].mStartTime = -1;
               }

               gWorkBuffer[ch] += sample;
            }

            mPlayheads[playhead].mOffset += sampleSpeed * mPlayheads[playhead].mSpeedTweak * mSample.GetSampleRateRatio();
            mPlayheads[playhead].mEnvelopeTime += gInvSampleRateMs / mPlayheads[playhead].mEnvelopeScale;
            mSamplesRemainingToProcess = bufferSize + abs(mWiden);
         }
      }

      int secondChannel = out->NumActiveChannels() == 1 ? 0 : 1;
      float left = gWorkBuffer[0];
      float right = gWorkBuffer[secondChannel];

      if (mPan + mPanInput != 0 && mOwner->mMonoOutput == false)
      {
         out->GetChannel(0)[i] = left * ofMap(mPan + mPanInput, 0, 1, 1, 0, true) + right * ofMap(mPan + mPanInput, -1, 0, 1, 0, true);
         out->GetChannel(secondChannel)[i] = right * ofMap(mPan + mPanInput, -1, 0, 0, 1, true) + left * ofMap(mPan + mPanInput, 0, 1, 0, 1, true);
      }
      else
      {
         out->GetChannel(0)[i] = left;
         if (secondChannel == 1)
            out->GetChannel(secondChannel)[i] = right;
      }

      time += gInvSampleRateMs;
   }   

   if (mSamplesRemainingToProcess > 0)
   {
      if (abs(mWiden) > 0 && mOwner->mMonoOutput == false)
      {
         mWidenerBuffer.SetNumChannels(2);
         for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
            mWidenerBuffer.WriteChunk(out->GetChannel(ch), bufferSize, ch);
         if (mWiden < 0)
            mWidenerBuffer.ReadChunk(out->GetChannel(1), bufferSize, abs(mWiden), 1);
         else
            mWidenerBuffer.ReadChunk(out->GetChannel(0), bufferSize, abs(mWiden), 0);
      }

      mSamplesRemainingToProcess -= bufferSize;

      return true;
   }
      
   return false;
}

int DrumPlayer::GetIndividualOutputIndex(int hitIndex)
{
   for (int i=0; i<mIndividualOutputs.size(); ++i)
   {
      if (mIndividualOutputs[i]->mHitIndex == hitIndex)
         return i;
   }
   return -1;
}

void DrumPlayer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(time))
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   pitch %= 24;
   if (pitch >= 0 && pitch < NUM_DRUM_HITS)
   {
      if (velocity > 0)
      {
         //reset all linked drum hits
         int playingId = mDrumHits[pitch].mLinkId;
         if (playingId != -1)
         {
            for (int i = 0; i < NUM_DRUM_HITS; ++i)
            {
               if (i != pitch && mDrumHits[i].mLinkId == playingId)
                  mDrumHits[i].StopLinked(time);
            }
         }

         //play this one
         mDrumHits[pitch].mVelocity = velocity / 127.0f;
         mDrumHits[pitch].mPanInput = modulation.pan;
         mDrumHits[pitch].mPitchBend = modulation.pitchBend;
         float startOffsetPercent = mDrumHits[pitch].mStartOffset;
         if (modulation.modWheel != nullptr)
            startOffsetPercent += modulation.modWheel->GetValue(0);
         mDrumHits[pitch].StartPlayhead(time, startOffsetPercent, velocity/127.0f);
      }
   }
}

void DrumPlayer::FilesDropped(std::vector<std::string> files, int x, int y)
{
   x -= 5;
   y -= 70;
   if (x<0 || y<0)
      return;
   x /= 70;
   y /= 70;
   
   File auditionDir(files[0]);

   if (auditionDir.isDirectory())
   {
      mAuditionDir = files[0];
      if (x < 4 && y < 4)
         mSelectedHitIdx = GetAssociatedSampleIndex(x,y);
      mAuditionSampleIdx = -1;
   }
   else
   {
      auditionDir = "";
      if (x < 4 && y < 4)
      {
         for (int i=0; i<files.size(); ++i)
         {
            int sampleIdx = GetAssociatedSampleIndex(x+i%4, y+i/4);
            if (sampleIdx != -1)
            {
               LoadSampleLock();
               mDrumHits[sampleIdx].mSample.Read(files[i].c_str());
               LoadSampleUnlock();
               mDrumHits[sampleIdx].mLinkId = -1;
               mDrumHits[sampleIdx].mVol = 1;
               mDrumHits[sampleIdx].mSpeed = 1;
               mDrumHits[sampleIdx].mPan = 0;
               mDrumHits[sampleIdx].mEnvelopeLength = mDrumHits[sampleIdx].mSample.LengthInSamples() * gInvSampleRateMs;

               mSelectedHitIdx = sampleIdx;
               UpdateVisibleControls();
            }
         }
      }
   }
}

void DrumPlayer::SampleDropped(int x, int y, Sample* sample)
{
   assert(sample);
   int numSamples = sample->LengthInSamples();
   
   if (numSamples <= 0)
      return;
   
   x -= 5;
   y -= 70;
   if (x<0 || y<0)
      return;
   x /= 70;
   y /= 70;
   
   mAuditionDir = "";
   if (x < 4 && y < 4)
   {
      int sampleIdx = GetAssociatedSampleIndex(x,y);
      if (sampleIdx != -1)
      {
         SetHitSample(sampleIdx, sample);

         mSelectedHitIdx = sampleIdx;
         UpdateVisibleControls();
      }
   }
}

void DrumPlayer::ImportSampleCuePoint(SamplePlayer* player, int sourceCueIndex, int destHitIndex)
{
   if (player != nullptr)
   {
      ChannelBuffer* data = player->GetCueSampleData(sourceCueIndex);
      Sample sample;
      sample.Create(data);
      SetHitSample(destHitIndex, &sample);
      delete data;
   }
}

void DrumPlayer::SetHitSample(int sampleIndex, Sample* sample)
{
   LoadSampleLock();
   mDrumHits[sampleIndex].mSample.CopyFrom(sample);
   LoadSampleUnlock();
   mDrumHits[sampleIndex].mLinkId = -1;
   mDrumHits[sampleIndex].mVol = 1;
   mDrumHits[sampleIndex].mSpeed = 1;
   mDrumHits[sampleIndex].mPan = 0;
   mDrumHits[sampleIndex].mEnvelopeLength = mDrumHits[sampleIndex].mSample.LengthInSamples() * gInvSampleRateMs;
}

void DrumPlayer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (!mEditMode)
      return;

   x -= 5;
   y -= 70;
   if (x<0 || y<0)
      return;
   x /= 70;
   y /= 70;
   if (x < 4 && y < 4)
   {
      int sampleIdx = GetAssociatedSampleIndex(x, y);
      if (sampleIdx != -1)
      {
         mSelectedHitIdx = sampleIdx;
         mAuditionDir = ofToDataPath("drums/"+mDrumHits[sampleIdx].mHitCategory);
         UpdateVisibleControls();
      }
   }
}

void DrumPlayer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int sampleIdx = GetAssociatedSampleIndex(x, y);
   if (sampleIdx >= 0)
   {
      if (velocity > 0 && mQuantizeInterval == kInterval_None)
      {
         PlayNote(gTime + gBufferSizeMs, sampleIdx, velocity * 127);
      }
      else
      {
         if (sampleIdx < mDrumHits.size())
         {
            if (velocity > 0)
               mDrumHits[sampleIdx].mButtonHeldVelocity = velocity * 127;
            else if (mNoteRepeat)
               mDrumHits[sampleIdx].mButtonHeldVelocity = 0;
         }
      }
   }
}

void DrumPlayer::OnTimeEvent(double time)
{
   for (int i = 0; i < (int)mDrumHits.size(); ++i)
   {
      if (mDrumHits[i].mButtonHeldVelocity > 0)
      {
         PlayNote(time, i, mDrumHits[i].mButtonHeldVelocity);
         if (!mNoteRepeat)
            mDrumHits[i].mButtonHeldVelocity = 0;
      }
   }
}

void DrumPlayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mSpeedSlider->Draw();
   mSpeedRandomizationSlider->Draw();
   mKitSelector->Draw();
   mEditCheckbox->Draw();
   mGridControlTarget->Draw();

   if (mEditMode)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(50, 50, 50, gModuleDrawAlpha);
      ofRect(300, 5, 145, 360);
      ofPopStyle();
      
      mMonoCheckbox->Draw();
      //mSaveButton->Draw();
      //mNewKitButton->Draw();
      //mNewKitNameEntry->Draw();
      mAuditionSlider->Draw();
      mShuffleButton->Draw();
      mQuantizeIntervalSelector->Draw();
      mNoteRepeatCheckbox->Draw();

      ofPushMatrix();
      ofPushStyle();
      ofTranslate(5, 70);
      for (int i=0; i<4; ++i)
      {
         for (int j=0; j<4; ++j)
         {
            int sampleIdx = GetAssociatedSampleIndex(i, j);

            ofSetColor(200, 100, 0, gModuleDrawAlpha);
            ofNoFill();
            ofRect(i * 70, j * 70, 70, 70);

            float alpha = sqrt(1 - mDrumHits[sampleIdx].GetPlayProgress(gTime));
            ofSetColor(200,100,0,gModuleDrawAlpha * alpha);
            ofFill();
            ofRect(i*70,j*70,70,70);
            
            if (sampleIdx == mSelectedHitIdx)
            {
               ofSetColor(0,200,255,gModuleDrawAlpha);
               ofNoFill();
               ofRect(i*70+1,j*70+1,68,68);
            }

            ofSetColor(255,255,255,gModuleDrawAlpha);
            gFont.DrawStringWrap(mDrumHits[sampleIdx].mSample.Name(), 12, i*70+5,j*70+10, 60);
         }
      }
      ofPopStyle();
      ofPopMatrix();

      for (size_t i = 0; i < mDrumHits.size(); ++i)
      {
         mDrumHits[i].mTestButton->Draw();
         mDrumHits[i].mRandomButton->Draw();
      }
      
      if (mSelectedHitIdx != -1)
         mDrumHits[mSelectedHitIdx].DrawUIControls();
   }
   
   float moduleW, moduleH;
   GetDimensions(moduleW, moduleH);
   for (int i=0; i<mIndividualOutputs.size(); ++i)
   {
      DrawTextNormal(ofToString(mIndividualOutputs[i]->mHitIndex), moduleW - 20, 10 + i*12);
      mIndividualOutputs[i]->UpdatePosition(i);
   }
}

void DrumPlayer::UpdateLights()
{
   const int kCols = 4;
   const int kRows = 4;
   for (int x=0; x<kCols; ++x)
   {
      for (int y=0; y<kRows; ++y)
      {
         int sampleIdx = GetAssociatedSampleIndex(x, y);
         Sample* sample = nullptr;
         if (sampleIdx != -1)
            sample = &(mDrumHits[sampleIdx].mSample);
         if (mGridControlTarget->GetGridController())
         {
            if (mDrumHits[sampleIdx].GetPlayProgress(gTime) < .75f)
               mGridControlTarget->GetGridController()->SetLight(x,y,kGridColor3Bright);
            else if (sample)
               mGridControlTarget->GetGridController()->SetLight(x,y,kGridColor3Dim);
            else
               mGridControlTarget->GetGridController()->SetLight(x,y,kGridColorOff);
         }
      }
   }
}

void DrumPlayer::OnControllerPageSelected()
{
   UpdateLights();
}

void DrumPlayer::DrumHit::DrawUIControls()
{
   float displayLength = mSample.LengthInSamples();
   if (mUseEnvelope)
      displayLength = MIN(mEnvelopeLength * gSampleRateMs, displayLength);
   ofPushMatrix();
   ofTranslate(mEnvelopeDisplay->GetPosition(true).x, mEnvelopeDisplay->GetPosition(true).y);
   if (!mOwner->mLoadingSamples)
   {
      mOwner->mLoadSamplesDrawMutex.lock();
      DrawAudioBuffer(135, 100, mSample.Data(), mStartOffset*displayLength, displayLength, mSample.GetPlayPosition());
      mOwner->mLoadSamplesDrawMutex.unlock();
   }
   ofPopMatrix();
   
   mVolSlider->Draw();
   mSpeedSlider->Draw();
   mLinkIdSlider->Draw();
   mHitCategoryDropdown->Draw();
   mPanSlider->Draw();
   mWidenSlider->Draw();
   mStartOffsetSlider->Draw();
   mIndividualOutputCheckbox->Draw();
   mUseEnvelopeCheckbox->Draw();
   if (mUseEnvelope)
   {
      mEnvelopeLengthSlider->SetExtents(10, mSample.LengthInSamples() * gInvSampleRateMs);
      mEnvelopeLengthSlider->Draw();
      mEnvelopeDisplay->SetMaxTime(mEnvelopeLength);
      mEnvelopeDisplay->SetOverrideDrawTime(mPlayheads[mCurrentPlayheadIndex].mEnvelopeTime);
      mEnvelopeDisplay->Draw();
   }
   mGrabSampleButton->Draw();
}

int DrumPlayer::GetAssociatedSampleIndex(int x, int y)
{
    if (x > 3)
    {
        // For long rows, overflow the x value vertically
        // This makes e.g. 8 pads on a single row usable for
        // two drumplayer rows
        y = y + (x / 4);
        x = x % 4;
    }
   int pos = x+(3-y)*4;
   if (pos < 16)
      return pos;
   return -1;
}

void DrumPlayer::SaveKits()
{
   ofxJSONElement root;

   Json::Value& kits = root["kits"];
   for (int i=0; i<mKits.size(); ++i)
   {
      Json::Value& kit = kits[i];
      
      if (i == mLoadedKit)
      {
         for (int j=0; j<NUM_DRUM_HITS; ++j)
         {
            //get relative path if it's in our data dir
            std::string path = mDrumHits[j].mSample.GetReadPath();
            ofStringReplace(path, File(ofToDataPath("")).getFullPathName().toStdString(), "");
            
            mKits[i].mSampleFiles[j] = path;
            mKits[i].mLinkIds[j] = mDrumHits[j].mLinkId;
            mKits[i].mVols[j] = mDrumHits[j].mVol;
            mKits[i].mSpeeds[j] = mDrumHits[j].mSpeed;
            mKits[i].mPans[j] = mDrumHits[j].mPan;
         }
      }
      
      for (int j=0; j<NUM_DRUM_HITS; ++j)
      {
         kit["samples"][j]["sample"] = mKits[i].mSampleFiles[j];
         kit["samples"][j]["linkid"] = mKits[i].mLinkIds[j];
         kit["samples"][j]["vol"] = mKits[i].mVols[j];
         kit["samples"][j]["speed"] = mKits[i].mSpeeds[j];
         kit["samples"][j]["pan"] = mKits[i].mPans[j];
      }
      kit["name"] = mKits[i].mName;
   }

   root.save(ofToDataPath("drums/drumkits.json"), true);
}

void DrumPlayer::ReadKits()
{
   ofxJSONElement root;
   root.open(ofToDataPath("drums/drumkits.json"));

   Json::Value& kits = root["kits"];
   mKits.resize(kits.size());
   for (int i=0; i<kits.size(); ++i)
   {
      try
      {
         Json::Value& kit = kits[i];
         int numHitsInFile = kit["samples"].size();
         for (int j=0; j<NUM_DRUM_HITS && j<numHitsInFile; ++j)
         {
            mKits[i].mSampleFiles[j] = kit["samples"][j]["sample"].asString();
            mKits[i].mLinkIds[j] = kit["samples"][j]["linkid"].asInt();
            if (kit["samples"][j]["vol"].isNull() == false)
               mKits[i].mVols[j] = kit["samples"][j]["vol"].asDouble();
            else
               mKits[i].mVols[j] = 1;
            if (kit["samples"][j]["speed"].isNull() == false)
               mKits[i].mSpeeds[j] = kit["samples"][j]["speed"].asDouble();
            else
               mKits[i].mSpeeds[j] = 1;
            if (kit["samples"][j]["pan"].isNull() == false)
               mKits[i].mPans[j] = kit["samples"][j]["pan"].asDouble();
            else
               mKits[i].mPans[j] = 0;
         }
         mKits[i].mName = kit["name"].asString();
      }
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }
}

void DrumPlayer::CreateKit()
{
   StoredDrumKit kit;

   kit.mName = mNewKitName;
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      kit.mSampleFiles[i] = mDrumHits[i].mSample.GetReadPath();
      kit.mLinkIds[i] = mDrumHits[i].mLinkId;
   }

   mKits.push_back(kit);
   mLoadedKit = (int)mKits.size() - 1;
   mKitSelector->AddLabel(kit.mName.c_str(), mLoadedKit);
}

void DrumPlayer::ShuffleKit()
{
   for (int j=0; j<NUM_DRUM_HITS; ++j)
   {
      mDrumHits[j].LoadRandomSample();
      mDrumHits[j].mVol *= ofRandom(.9f,1.1f);
      mDrumHits[j].mSpeed *= ofRandom(.9f,1.1f);
      mDrumHits[j].mPan = ofRandom(-1.0f,1.0f);
   }
}

void DrumPlayer::GetModuleDimensions(float& width, float& height)
{
   if (mEditMode)
   {
      width = 450;
      height = 370;
   }
   else
   {
      if (mIndividualOutputs.size() > 0)
         width = 150;
      else
         width = 110;
      height = 70;
   }
}

void DrumPlayer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mAuditionSlider)
   {
      mAuditionSampleIdx += mAuditionInc>0?-1:1;
      mAuditionInc = 0;
      File dir(mAuditionDir);
      Array<File> files;
      dir.findChildFiles(files, File::findFiles, false);
      if (files.size() > 0)
      {
         mAuditionSampleIdx = ofClamp(mAuditionSampleIdx,0,files.size()-1);
         
         std::string file = files[mAuditionSampleIdx].getFullPathName().toStdString();
         if (mSelectedHitIdx >= 0 && mSelectedHitIdx < NUM_DRUM_HITS)
         {
            LoadSampleLock();
            mDrumHits[mSelectedHitIdx].mSample.Read(file.c_str());
            LoadSampleUnlock();
            mDrumHits[mSelectedHitIdx].StartPlayhead(gTime, 0, 1);
            mDrumHits[mSelectedHitIdx].mVelocity = .5f;
            mDrumHits[mSelectedHitIdx].mEnvelopeLength = mDrumHits[mSelectedHitIdx].mSample.LengthInSamples() * gInvSampleRateMs;
         }
      }
   }
}

void DrumPlayer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void DrumPlayer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mKitSelector)
      LoadKit(mLoadedKit);
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      if (list == mDrumHits[i].mHitCategoryDropdown)
         mDrumHits[i].mHitCategory = mDrumHits[i].mHitCategoryDropdown->GetLabel(mDrumHits[i].mHitCategoryIndex);
   }

   if (list == mQuantizeIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mQuantizeInterval;
   }
}

void DrumPlayer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEditCheckbox)
      UpdateVisibleControls();
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      if (checkbox == mDrumHits[i].mIndividualOutputCheckbox)
      {
         int outputIndex = GetIndividualOutputIndex(i);
         if (mDrumHits[i].mHasIndividualOutput)
         {
            if (outputIndex == -1)
               mIndividualOutputs.push_back(new IndividualOutput(this, i));
         }
         else
         {
            if (outputIndex != -1)
            {
               RemovePatchCableSource(mIndividualOutputs[outputIndex]->mPatchCableSource);
               delete mIndividualOutputs[outputIndex];
               mIndividualOutputs.erase(mIndividualOutputs.begin() + outputIndex);
            }
         }
      }
   }
}

void DrumPlayer::ButtonClicked(ClickButton* button)
{
   /*if (button == mSaveButton)
      SaveKits();
   if (button == mNewKitButton)
      CreateKit();*/
   if (button == mShuffleButton)
      ShuffleKit();
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      if (button == mDrumHits[i].mTestButton)
         PlayNote(gTime + gBufferSizeMs, i, 127);
      if (button == mDrumHits[i].mRandomButton)
         mDrumHits[i].LoadRandomSample();
      if (button == mDrumHits[i].mGrabSampleButton)
         mDrumHits[i].GrabSample();
   }
}

void DrumPlayer::DrumHit::LoadRandomSample()
{
   File dir(ofToDataPath("drums/"+mHitCategory));
   Array<File> files;
   for (auto file : dir.findChildFiles(File::findFiles, false))
   {
      if (file.getFileName()[0] != '.')
         files.add(file);
   }

   if (files.size() > 0)
   {
      std::string file = files[gRandom() % files.size()].getFullPathName().toStdString();
      
      mOwner->LoadSampleLock();
      mSample.Read(file.c_str());
      mOwner->LoadSampleUnlock();
      //mSample.Play(gTime, mSpeed, 0);
      //mVelocity = .5f;
      mEnvelopeLength = mSample.LengthInSamples() * gInvSampleRateMs;
   }
}

void DrumPlayer::DrumHit::GrabSample()
{
   TheSynth->GrabSample(mSample.Data(), mSample.Name());
}

void DrumPlayer::TextEntryComplete(TextEntry* entry)
{
}

std::vector<IUIControl*> DrumPlayer::ControlsToNotSetDuringLoadState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mKitSelector);
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      ignore.push_back(mDrumHits[i].mHitCategoryDropdown);
   return ignore;
}

void DrumPlayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void DrumPlayer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   //LoadKit(0);
}

namespace
{
   const int kSaveStateRev = 1;
}

void DrumPlayer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      mDrumHits[i].mSample.SaveState(out);
      out << mDrumHits[i].mLinkId;
      out << mDrumHits[i].mVol;
      out << mDrumHits[i].mSpeed;
      out << mDrumHits[i].mHitCategory;
   }
}

void DrumPlayer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      mDrumHits[i].mSample.LoadState(in);
      in >> mDrumHits[i].mLinkId;
      in >> mDrumHits[i].mVol;
      in >> mDrumHits[i].mSpeed;
      if (rev >= 1)
      {
         in >> mDrumHits[i].mHitCategory;
         mDrumHits[i].UpdateHitDirectoryDropdown();
      }
   }

   mNeedSetup = false;
}
