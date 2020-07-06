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

DrumPlayer::DrumPlayer()
: mSpeed(1)
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
, mAuditionPadIdx(0)
, mNewKitNameEntry(nullptr)
, mLoadingSamples(false)
, mShuffleButton(nullptr)
, mSelectedHitIdx(0)
, mOutputBuffer(gBufferSize)
, mMonoOutput(false)
, mMonoCheckbox(nullptr)
, mGridController(nullptr)
, mNoteInputBuffer(this)
{
   ReadKits();
   
   strcpy(mNewKitName, "new");
}

void DrumPlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",4,2,100,15,&mVolume,0,2);
   mSpeedSlider = new FloatSlider(this,"speed",4,18,100,15,&mSpeed,0.2f,3);
   mKitSelector = new DropdownList(this,"kit",4,50,&mLoadedKit);
   mEditCheckbox = new Checkbox(this,"edit",73,52,&mEditMode);
   mSaveButton = new ClickButton(this,"save current",200,22);
   mNewKitButton = new ClickButton(this,"new kit", 200, 4);
   mNewKitNameEntry = new TextEntry(this,"kitname",200, 40,7,mNewKitName);
   mAuditionSlider = new FloatSlider(this,"aud",140,50,40,15,&mAuditionInc,-1,1,0);
   mMonoCheckbox = new Checkbox(this,"mono",4,34,&mMonoOutput);
   mShuffleButton = new ClickButton(this,"shuffle",140,34);
   mGridController = new GridController(this, "grid", 60, 34);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mDrumHits[i].CreateUIControls(this, i);
   
   for (int i=0; i<mKits.size(); ++i)
      mKitSelector->AddLabel(mKits[i].mName.c_str(), i);
   
   UpdateVisibleControls();
   
   GetPatchCableSource()->SetManualSide(PatchCableSource::kBottom);
}

void DrumPlayer::DrumHit::CreateUIControls(DrumPlayer* owner, int index)
{
   UIBLOCK(310,37);
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER owner //change owner
   FLOATSLIDER_DIGITS(mVolSlider, ("vol "+ofToString(index)).c_str(),&mVol,0,1,2);
   FLOATSLIDER_DIGITS(mSpeedSlider, ("speed "+ofToString(index)).c_str(),&mSpeed,.2f,3,2);
   FLOATSLIDER(mPanSlider, ("pan "+ofToString(index)).c_str(),&mPan,-1,1);
   INTSLIDER(mWidenSlider, ("widen "+ofToString(index)).c_str(),&mWiden,-150,150);
   CHECKBOX(mIndividualOutputCheckbox, ("single out "+ofToString(index)).c_str(),&mHasIndividualOutput);
   INTSLIDER(mLinkIdSlider, ("linkid " + ofToString(index)).c_str(), &mLinkId, -1, 5);
   CHECKBOX(mUseEnvelopeCheckbox, ("envelope "+ofToString(index)).c_str(),&mUseEnvelope);
   FLOATSLIDER(mEnvelopeLengthSlider, ("view ms "+ofToString(index)).c_str(),&mEnvelopeLength,10,2000);
   DROPDOWN(mHitCategoryDropdown,("hitcategory"+ofToString(index)).c_str(),&mHitCategoryIndex,100);
   UICONTROL_CUSTOM(mEnvelopeDisplay, new ADSRDisplay(UICONTROL_BASICS(("envelopedisplay "+ofToString(index)).c_str()),135, 100,&mEnvelope));
   ENDUIBLOCK0();
#undef UIBLOCK_OWNER
#define UIBLOCK_OWNER this //reset
   
   int x = 5 + (index % 4) * 70;
   int y = 70 + (3-(index / 4)) * 70;
   mTestButton = new ClickButton(owner,("test "+ofToString(index)).c_str(),x+5,y+40);
   mRandomButton = new ClickButton(owner,("random "+ofToString(index)).c_str(),x+5,y+53);
   
   string hitCategory = "Percussion";
   if (index == 0)
      hitCategory = "Kick";
   if (index == 1)
      hitCategory = "Snare";
   if (index == 2 || index == 6)
      hitCategory = "Hihat";
   if (index == 3)
      hitCategory = "Ride";
   if (index == 4)
      hitCategory = "Shaker";
   if (index == 5)
      hitCategory = "Crash";
   if (index == 6)
      hitCategory = "Percussion";
   if (index == 7)
      hitCategory = "Clap";
   mHitCategory = hitCategory;
   
   UpdateHitDirectoryDropdown();
   
   mOwner = owner;
}

namespace
{
   static list<string> sHitDirectories;
}

//static
void DrumPlayer::SetUpHitDirectories()
{
   sHitDirectories.clear();
   File parentDirectory(ofToDataPath("drums/hits"));
   Array<File> hitDirs;
   parentDirectory.findChildFiles(hitDirs, File::findDirectories, true);
   for (auto dir : hitDirs)
   {
      Array<File> filesInDir;
      dir.findChildFiles(filesInDir, File::findFiles, false);
      if (filesInDir.size() > 0)
         sHitDirectories.push_back(dir.getRelativePathFrom(parentDirectory).toStdString());
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
   UpdateLights();
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
   mIndividualOutputCheckbox->SetShowing(showing);
   mLinkIdSlider->SetShowing(showing);
   mHitCategoryDropdown->SetShowing(showing);
}

DrumPlayer::~DrumPlayer()
{
   for (int i=0; i<mIndividualOutputs.size(); ++i)
      delete mIndividualOutputs[i];
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

string DrumPlayer::GetDrumHitName(int index)
{
   switch (index)
   {
      case 0:
         return "kick";
      case 1:
         return "rim";
      case 2:
         return "closed";
      case 3:
         return "ride";
      case 4:
         return "snare";
      case 5:
         return "crash";
      case 6:
         return "open";
      case 7:
         return "stick";
      case 8:
         return "floor";
      case 9:
         return "low";
      case 10:
         return "mid";
      case 11:
         return "hi";
      case 12:
         return "misc1";
      case 13:
         return "misc2";
      case 14:
         return "misc3";
      case 15:
         return "misc4";
   }
   return ofToString(index);
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
   
   float volSq = mVolume * mVolume;
   
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
                  if (GetTarget(targetIndex))
                     Add(GetTarget(targetIndex)->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
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
   
   for (int ch=0; ch<numChannels; ++ch)
   {
      GetVizBuffer()->WriteChunk(mOutputBuffer.GetChannel(ch), bufferSize, ch);
   
      if (GetTarget())
         Add(GetTarget()->GetBuffer()->GetChannel(ch), mOutputBuffer.GetChannel(ch), bufferSize);
   }
}

bool DrumPlayer::DrumHit::Process(double time, float speed, float vol, ChannelBuffer* out, int bufferSize)
{
   mSample.SetRate(speed * mSpeed);
   
   if (mSample.ConsumeData(time, out, bufferSize, true))
   {
      double timeHit = time;
      for (int j=0; j<bufferSize; ++j)
      {
         for (int ch=0; ch<out->NumActiveChannels(); ++ch)
         {
            float sample = out->GetChannel(ch)[j] * mVelocity * vol * mVol * mVol;
            if (mUseEnvelope)
               sample *= mEnvelope.Value(timeHit);
            out->GetChannel(ch)[j] = sample;
         }
         
         if (mPan + mPanInput != 0 && mOwner->mMonoOutput == false)
         {
            int secondChannel = out->NumActiveChannels() == 1 ? 0 : 1;
            
            float left = out->GetChannel(0)[j];
            float right = out->GetChannel(secondChannel)[j];
            out->GetChannel(0)[j] = left * ofMap(mPan + mPanInput, 0, 1, 1, 0, true) + right * ofMap(mPan + mPanInput, -1, 0, 1, 0, true);
            out->GetChannel(secondChannel)[j] = right * ofMap(mPan + mPanInput, -1, 0, 0, 1, true) + left * ofMap(mPan + mPanInput, 0, 1, 0, 1, true);
         }
         
         timeHit += gInvSampleRateMs;
      }
      
      mSamplesRemainingToProcess = bufferSize + abs(mWiden);
   }
   else
   {
      out->Clear();
   }

   if (mSamplesRemainingToProcess > 0)
   {
      if (abs(mWiden) > 0 && mOwner->mMonoOutput == false)
      {
         mWidenerBuffer.SetNumChannels(2);
         for (int ch=0; ch<out->NumActiveChannels(); ++ch)
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
               if (mDrumHits[i].mLinkId == playingId && mDrumHits[i].mSample.GetPlayPosition() > 100)
                  mDrumHits[i].mSample.Reset();
            }
         }

         //play this one
         mDrumHits[pitch].mSample.Play(time, mSpeed * ofRandom(.99f,1.01f), 0);
         mDrumHits[pitch].mVelocity = velocity / 127.0f;
         mDrumHits[pitch].mPanInput = modulation.pan;
         mDrumHits[pitch].mEnvelope.Start(time, 1);
      }
   }
}

void DrumPlayer::FilesDropped(vector<string> files, int x, int y)
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
         mAuditionPadIdx = GetAssociatedSampleIndex(x,y);
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
         LoadSampleLock();
         mDrumHits[sampleIdx].mSample.Create(sample->Data());
         LoadSampleUnlock();
         mDrumHits[sampleIdx].mLinkId = -1;
         mDrumHits[sampleIdx].mVol = 1;
         mDrumHits[sampleIdx].mSpeed = 1;
         mDrumHits[sampleIdx].mPan = 0;
      }
   }
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
         UpdateVisibleControls();
      }
   }
}

void DrumPlayer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int sampleIdx = GetAssociatedSampleIndex(x, y);
   if (sampleIdx != -1)
   {
      PlayNote(gTime, sampleIdx, velocity*127);
   }
}

void DrumPlayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mSpeedSlider->Draw();
   mKitSelector->Draw();
   mEditCheckbox->Draw();
   mMonoCheckbox->Draw();
   mGridController->Draw();
   
   for (int i=0; i<mIndividualOutputs.size(); ++i)
      DrawTextNormal(GetDrumHitName(mIndividualOutputs[i]->mHitIndex), 110, 10 + i*12);

   if (mEditMode)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(50, 50, 50, gModuleDrawAlpha);
      ofRect(300, 5, 145, 360);
      ofPopStyle();
      
      mSaveButton->Draw();
      mNewKitButton->Draw();
      mNewKitNameEntry->Draw();
      mAuditionSlider->Draw();
      mShuffleButton->Draw();

      ofPushMatrix();
      ofPushStyle();
      ofTranslate(5, 70);
      for (int i=0; i<4; ++i)
      {
         for (int j=0; j<4; ++j)
         {
            int sampleIdx = GetAssociatedSampleIndex(i, j);
            ofSetColor(200,100,0,gModuleDrawAlpha);
            Sample* sample = nullptr;
            if (sampleIdx != -1)
               sample = &(mDrumHits[sampleIdx].mSample);
            if (sample &&
                sample->IsPlaying() &&
                sample->GetPlayPosition() < gSampleRate * .25f)
               ofFill();
            else
               ofNoFill();
            ofRect(i*70,j*70,70,70);
            
            if (sampleIdx == mSelectedHitIdx)
            {
               ofSetColor(0,200,255,gModuleDrawAlpha);
               ofNoFill();
               ofRect(i*70+1,j*70+1,68,68);
            }

            ofSetColor(255,255,255,gModuleDrawAlpha);
            if (sample)
            {
               gFont.DrawStringWrap(sample->Name(), 12, i*70+5,j*70+10, 60);
            }
         }
      }
      ofPopStyle();
      ofPopMatrix();
      
      if (mSelectedHitIdx != -1)
         mDrumHits[mSelectedHitIdx].DrawUIControls();
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
         if (sample &&
             sample->IsPlaying() &&
             sample->GetPlayPosition() < gSampleRate * .25f)
            mGridController->SetLight(x,y,kGridColor3Bright);
         else if (sample)
            mGridController->SetLight(x,y,kGridColor3Dim);
         else
            mGridController->SetLight(x,y,kGridColorOff);
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
      displayLength = mEnvelopeLength * gSampleRateMs;
   ofPushMatrix();
   ofTranslate(mEnvelopeDisplay->GetPosition(true).x, mEnvelopeDisplay->GetPosition(true).y);
   if (!mOwner->mLoadingSamples)
   {
      mOwner->mLoadSamplesDrawMutex.lock();
      DrawAudioBuffer(135, 100, mSample.Data(), 0, displayLength, mSample.GetPlayPosition());
      mOwner->mLoadSamplesDrawMutex.unlock();
   }
   ofPopMatrix();
   
   mVolSlider->Draw();
   mSpeedSlider->Draw();
   mTestButton->Draw();
   mRandomButton->Draw();
   mLinkIdSlider->Draw();
   mHitCategoryDropdown->Draw();
   mPanSlider->Draw();
   mWidenSlider->Draw();
   mIndividualOutputCheckbox->Draw();
   mUseEnvelopeCheckbox->Draw();
   if (mUseEnvelope)
   {
      mEnvelopeLengthSlider->SetExtents(10, mSample.LengthInSamples() * gInvSampleRateMs);
      mEnvelopeLengthSlider->Draw();
      mEnvelopeDisplay->SetMaxTime(mEnvelopeLength);
      mEnvelopeDisplay->Draw();
   }
}

int DrumPlayer::GetAssociatedSampleIndex(int x, int y)
{
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
            string path = mDrumHits[j].mSample.GetReadPath();
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

   root.save(ofToDataPath("drums/drums.json"), true);
}

void DrumPlayer::ReadKits()
{
   ofxJSONElement root;
   root.open(ofToDataPath("drums/drums.json"));

   Json::Value& kits = root["kits"];
   mKits.resize(kits.size());
   for (int i=0; i<kits.size(); ++i)
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
         
         string file = files[mAuditionSampleIdx].getFullPathName().toStdString();
         if (mAuditionPadIdx >= 0 && mAuditionPadIdx < NUM_DRUM_HITS)
         {
            LoadSampleLock();
            mDrumHits[mAuditionPadIdx].mSample.Read(file.c_str());
            LoadSampleUnlock();
            mDrumHits[mAuditionPadIdx].mSample.Play(gTime, mSpeed, 0);
            mDrumHits[mAuditionPadIdx].mVelocity = .5f;
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
               mIndividualOutputs.push_back(new IndividualOutput(this, i, (int)mIndividualOutputs.size()));
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
   if (button == mSaveButton)
      SaveKits();
   if (button == mNewKitButton)
      CreateKit();
   if (button == mShuffleButton)
      ShuffleKit();
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      if (button == mDrumHits[i].mTestButton)
         PlayNote(gTime, i, 127);
      if (button == mDrumHits[i].mRandomButton)
         mDrumHits[i].LoadRandomSample();
   }
}

void DrumPlayer::DrumHit::LoadRandomSample()
{
   File dir(ofToDataPath("drums/hits/"+mHitCategory));
   Array<File> files;
   dir.findChildFiles(files, File::findFiles, false);
   if (files.size() > 0)
   {
      string file = files[rand() % files.size()].getFullPathName().toStdString();
      
      mOwner->LoadSampleLock();
      mSample.Read(file.c_str());
      mOwner->LoadSampleUnlock();
      //mSample.Play(gTime, mSpeed, 0);
      //mVelocity = .5f;
   }
}

void DrumPlayer::TextEntryComplete(TextEntry* entry)
{
}

vector<IUIControl*> DrumPlayer::ControlsToNotSetDuringLoadState() const
{
   vector<IUIControl*> ignore;
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
   LoadKit(0);
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
}
