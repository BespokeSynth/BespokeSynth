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
#include "LooperRecorder.h"
#include "ModularSynth.h"
#include "MidiController.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"

DrumPlayer::DrumPlayer()
: mSpeed(1)
, mRecordDrums(false)
, mRecordDrumsCheckbox(nullptr)
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
, mShuffleSpeedsButton(nullptr)
, mSelectedHitIdx(0)
{
   ReadKits();
   
   mOutputBuffer = new float[gBufferSize];
   
   strcpy(mNewKitName, "new");
}

void DrumPlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRecordDrumsCheckbox = new Checkbox(this,"rec",70,34,&mRecordDrums);
   mVolSlider = new FloatSlider(this,"vol",4,2,100,15,&mVolume,0,2);
   mSpeedSlider = new FloatSlider(this,"speed",4,18,100,15,&mSpeed,0.2f,3);
   mKitSelector = new DropdownList(this,"kit",4,50,&mLoadedKit);
   mEditCheckbox = new Checkbox(this,"edit",73,52,&mEditMode);
   mSaveButton = new ClickButton(this,"save current",200,22);
   mNewKitButton = new ClickButton(this,"new kit", 200, 4);
   mNewKitNameEntry = new TextEntry(this,"kitname",200, 40,7,mNewKitName);
   mAuditionSlider = new FloatSlider(this,"aud",140,50,40,15,&mAuditionInc,-1,1,0);
   mShuffleSpeedsButton = new ClickButton(this,"shuffle",4,34);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mDrumHits[i].CreateUIControls(this, i);
   
   for (int i=0; i<mKits.size(); ++i)
      mKitSelector->AddLabel(mKits[i].mName.c_str(), i);
   
   UpdateVisibleControls();
}

void DrumPlayer::DrumHit::CreateUIControls(DrumPlayer* owner, int index)
{
   mVolSlider = new FloatSlider(owner,("vol "+ofToString(index)).c_str(),310,37,100,15,&mVol,0,1,2);
   mSpeedSlider = new FloatSlider(owner,("speed "+ofToString(index)).c_str(),-1,-1,100,15,&mSpeed,.2f,3,2);
   mUseEnvelopeCheckbox = new Checkbox(owner,("envelope "+ofToString(index)).c_str(),-1,-1,&mUseEnvelope);
   mEnvelopeDisplay = new ADSRDisplay(owner,("envelopedisplay "+ofToString(index)).c_str(),-1,-1,100,40,&mEnvelope);
   mIndividualOutputCheckbox = new Checkbox(owner,("individual output "+ofToString(index)).c_str(),-1,-1,&mHasIndividualOutput);
   
   mSpeedSlider->PositionTo(mVolSlider, kAnchorDirection_Below);
   mUseEnvelopeCheckbox->PositionTo(mSpeedSlider, kAnchorDirection_Below);
   mEnvelopeDisplay->PositionTo(mUseEnvelopeCheckbox, kAnchorDirection_Below);
   mIndividualOutputCheckbox->PositionTo(mEnvelopeDisplay, kAnchorDirection_Below);
   
   int x = 5 + (index % 4) * 70;
   int y = 70 + (3-(index / 4)) * 70;
   mTestButton = new ClickButton(owner,("test "+ofToString(index)).c_str(),x+5,y+53);
   
   mOwner = owner;
}

void DrumPlayer::UpdateVisibleControls()
{
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mDrumHits[i].SetUIControlsShowing(i == mSelectedHitIdx);
}

void DrumPlayer::DrumHit::SetUIControlsShowing(bool showing)
{
   mVolSlider->SetShowing(showing);
   mSpeedSlider->SetShowing(showing);
   mUseEnvelopeCheckbox->SetShowing(showing);
   mEnvelopeDisplay->SetShowing(showing);
   mIndividualOutputCheckbox->SetShowing(showing);
}

DrumPlayer::~DrumPlayer()
{
   delete[] mOutputBuffer;
   for (int i=0; i<mIndividualOutputs.size(); ++i)
      delete mIndividualOutputs[i];
}

void DrumPlayer::LoadKit(int kit)
{
   if (kit >= 0 && kit < mKits.size())
   {
      // Maschine samples are in /Users/Shared/Maschine Library/Samples
      mLoadedKit = kit;

      mLoadSamplesMutex.lock();
      mLoadingSamples = true;
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         mDrumHits[i].mSample.Read(mKits[kit].mSampleFiles[i].c_str());
         mDrumHits[i].mLinkId = mKits[kit].mLinkIds[i];
         mDrumHits[i].mVol = mKits[kit].mVols[i];
         mDrumHits[i].mSpeed = mKits[kit].mSpeeds[i];
      }
      mLoadingSamples = false;
      mLoadSamplesMutex.unlock();
   }
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
   Profiler profiler("DrumPlayer");
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = gBufferSize;
   
   float volSq = mVolume * mVolume;
   
   Clear(mOutputBuffer, bufferSize);
   
   if (!mLoadingSamples)
   {
      mLoadSamplesMutex.lock();
      mLoadingSamples = true;
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         int individualOutputIndex = GetIndividualOutputIndex(i);
         if (mDrumHits[i].Process(time, mSpeed, volSq, &gWorkChannelBuffer, bufferSize))
         {
            if (individualOutputIndex != -1)
            {
               int targetIndex = individualOutputIndex + 1;
               if (GetTarget(targetIndex))
                  Add(GetTarget(targetIndex)->GetBuffer()->GetChannel(0), gWorkChannelBuffer.GetChannel(0), bufferSize);
               mIndividualOutputs[individualOutputIndex]->mVizBuffer->WriteChunk(gWorkChannelBuffer.GetChannel(0), bufferSize, 0);
            }
            else
            {
               Add(mOutputBuffer, gWorkChannelBuffer.GetChannel(0), bufferSize);
            }
         }
         else
         {
            if (individualOutputIndex != -1)
               mIndividualOutputs[individualOutputIndex]->mVizBuffer->WriteChunk(gZeroBuffer, bufferSize, 0);
         }
      }
      mLoadingSamples = false;
      mLoadSamplesMutex.unlock();
   }
   
   GetVizBuffer()->WriteChunk(mOutputBuffer, bufferSize, 0);
   
   if (GetTarget())
      Add(GetTarget()->GetBuffer()->GetChannel(0), mOutputBuffer, bufferSize);
}

bool DrumPlayer::DrumHit::Process(double time, float speed, float vol, ChannelBuffer* out, int bufferSize)
{
   mSample.SetRate(speed * mSpeed);
   if (mSample.ConsumeData(gWorkBuffer, bufferSize, true))
   {
      double timeHit = time;
      for (int j=0; j<bufferSize; ++j)
      {
         float sample = gWorkBuffer[j] * mVelocity * vol * mVol * mVol;
         if (mUseEnvelope)
            sample *= mEnvelope.Value(timeHit);
         if (mUseFilter)
            sample = mFilter.Filter(sample);
         out->GetChannel(0)[j] = sample;
         timeHit += gInvSampleRateMs;
      }
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

void DrumPlayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   pitch %= 24;
   if (pitch >= 0 && pitch < NUM_DRUM_HITS)
   {
      if (velocity > 0)
      {
         if (mDrumHits[pitch].mSample.GetPlayPosition() <= 0 ||
             mDrumHits[pitch].mSample.GetPlayPosition() > 2000.0f / mDrumHits[pitch].mSample.GetSampleRateRatio())
         {
            //reset all linked drum hits
            int playingId = mDrumHits[pitch].mLinkId;
            for (int i=0; i<NUM_DRUM_HITS; ++i)
            {
               if (mDrumHits[i].mLinkId == playingId && mDrumHits[i].mSample.GetPlayPosition() > 100)
                  mDrumHits[i].mSample.Reset();
            }

            //play this one
            mDrumHits[pitch].mSample.Play(mSpeed * ofRandom(.99f,1.01f));
            mDrumHits[pitch].mVelocity = velocity / 127.0f;
            mDrumHits[pitch].mEnvelope.Start(time, 1);
            
            if (pitch == 2)
               TheTransport->OnDrumEvent(kInterval_Hat);
            if (pitch == 1)
               TheTransport->OnDrumEvent(kInterval_Snare);
            if (pitch == 0)
               TheTransport->OnDrumEvent(kInterval_Kick);
         }
         
         if (mRecordDrums)
         {
            mLooperRecorder->StartFreeRecord();
         }
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
      static int sLoadId = 100;
      if (x < 4 && y < 4)
      {
         for (int i=0; i<files.size(); ++i)
         {
            int sampleIdx = GetAssociatedSampleIndex(x+i%4, y+i/4);
            if (sampleIdx != -1)
            {
               mLoadSamplesMutex.lock();
               mLoadingSamples = true;
               mDrumHits[sampleIdx].mSample.Read(files[i].c_str());
               mLoadingSamples = false;
               mLoadSamplesMutex.unlock();
               mDrumHits[sampleIdx].mLinkId = sLoadId;
               mDrumHits[sampleIdx].mVol = 1;
               mDrumHits[sampleIdx].mSpeed = 1;
               ++sLoadId;
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
   static int sLoadId = 100;
   if (x < 4 && y < 4)
   {
      int sampleIdx = GetAssociatedSampleIndex(x,y);
      if (sampleIdx != -1)
      {
         mLoadSamplesMutex.lock();
         mLoadingSamples = true;
         mDrumHits[sampleIdx].mSample.Create(sample->Data(), sample->LengthInSamples());
         mLoadingSamples = false;
         mLoadSamplesMutex.unlock();
         mDrumHits[sampleIdx].mLinkId = sLoadId;
         mDrumHits[sampleIdx].mVol = 1;
         mDrumHits[sampleIdx].mSpeed = 1;
         ++sLoadId;
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

void DrumPlayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolSlider->Draw();
   mSpeedSlider->Draw();
   mKitSelector->Draw();
   mEditCheckbox->Draw();
   mRecordDrumsCheckbox->Draw();
   mShuffleSpeedsButton->Draw();
   
   for (int i=0; i<mIndividualOutputs.size(); ++i)
      DrawText(GetDrumHitName(mIndividualOutputs[i]->mHitIndex), 110, 10 + i*12);

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
               gFont.DrawStringWrap(GetDrumHitName(GetAssociatedSampleIndex(i, j)) + ":\n" + sample->Name(), 15, i*70+5,j*70+10, 60);
            }
         }
      }
      ofPopStyle();
      ofPopMatrix();
      
      for (int i=0; i<NUM_DRUM_HITS; ++i)
         mDrumHits[i].DrawUIControls();
      
      ofPushMatrix();
      ofTranslate(305, 200);
      DrawAudioBuffer(135, 100, mDrumHits[mSelectedHitIdx].mSample.Data(), 0, mDrumHits[mSelectedHitIdx].mSample.LengthInSamples(), mDrumHits[mSelectedHitIdx].mSample.GetPlayPosition());
      ofPopMatrix();
   }
}

void DrumPlayer::DrumHit::DrawUIControls()
{
   mVolSlider->Draw();
   mSpeedSlider->Draw();
   mTestButton->Draw();
   mUseEnvelopeCheckbox->Draw();
   mEnvelopeDisplay->Draw();
   mIndividualOutputCheckbox->Draw();
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
         }
      }
      
      for (int j=0; j<NUM_DRUM_HITS; ++j)
      {
         kit["samples"][j]["sample"] = mKits[i].mSampleFiles[j];
         kit["samples"][j]["linkid"] = mKits[i].mLinkIds[j];
         kit["samples"][j]["vol"] = mKits[i].mVols[j];
         kit["samples"][j]["speed"] = mKits[i].mSpeeds[j];
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
   mLoadedKit = mKits.size() - 1;
   mKitSelector->AddLabel(kit.mName.c_str(), mLoadedKit);
}

void DrumPlayer::ShuffleSpeeds()
{
   for (int j=0; j<NUM_DRUM_HITS; ++j)
   {
      mDrumHits[j].mVol *= ofRandom(.9f,1.1f);
      mDrumHits[j].mSpeed *= ofRandom(.9f,1.1f);
   }
}

void DrumPlayer::GetModuleDimensions(int& width, int& height)
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
      DirectoryIterator dir(File(mAuditionDir), false);
      //TODO_PORT(Ryan)
      /*if (mAuditionDir.numFiles() > 0)
      {
         mAuditionSampleIdx = ofClamp(mAuditionSampleIdx,0,mAuditionDir.numFiles()-1);
         string file = mAuditionDir.getFile(mAuditionSampleIdx).path();
         if (mAuditionPadIdx >= 0 && mAuditionPadIdx < NUM_DRUM_HITS)
         {
            mLoadSamplesMutex.lock();
            mLoadingSamples = true;
            mSamples[mAuditionPadIdx].Read(file.c_str());
            mLoadingSamples = false;
            mLoadSamplesMutex.unlock();
            mSamples[mAuditionPadIdx].Play(mSpeed);
            mVelocity[mAuditionPadIdx] = .5f;
         }
      }*/
   }
}

void DrumPlayer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void DrumPlayer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mKitSelector)
      LoadKit(mLoadedKit);
}

void DrumPlayer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mRecordDrumsCheckbox)
   {
      if (mRecordDrums == false)
         mLooperRecorder->EndFreeRecord();
   }
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      if (checkbox == mDrumHits[i].mIndividualOutputCheckbox)
      {
         int outputIndex = GetIndividualOutputIndex(i);
         if (mDrumHits[i].mHasIndividualOutput)
         {
            if (outputIndex == -1)
               mIndividualOutputs.push_back(new IndividualOutput(this, i, mIndividualOutputs.size()));
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

void DrumPlayer::ButtonClicked(ClickButton *button)
{
   if (button == mSaveButton)
      SaveKits();
   if (button == mNewKitButton)
      CreateKit();
   if (button == mShuffleSpeedsButton)
      ShuffleSpeeds();
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      if (button == mDrumHits[i].mTestButton)
         PlayNote(gTime, i, 127);
   }
}

void DrumPlayer::TextEntryComplete(TextEntry* entry)
{
}

void DrumPlayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("looperrecorder",moduleInfo,"",FillDropdown<LooperRecorder*>);
   mModuleSaveData.LoadInt("drumkit", moduleInfo, 0, 0, mKits.size()-1, true);

   SetUpFromSaveData();
}

void DrumPlayer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetLooperRecorder(dynamic_cast<LooperRecorder*>(TheSynth->FindModule(mModuleSaveData.GetString("looperrecorder"),false)));
   LoadKit(ofClamp(mModuleSaveData.GetInt("drumkit"),0,mKits.size()-1));
}

namespace
{
   const int kSaveStateRev = 0;
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
   }
}

void DrumPlayer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      mDrumHits[i].mSample.LoadState(in);
      in >> mDrumHits[i].mLinkId;
      in >> mDrumHits[i].mVol;
      in >> mDrumHits[i].mSpeed;
   }
}


