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
, mRecordDrumsCheckbox(NULL)
, mVolume(1)
, mLoadedKit(0)
, mVolSlider(NULL)
, mSpeedSlider(NULL)
, mKitSelector(NULL)
, mEditMode(false)
, mEditCheckbox(NULL)
, mSaveButton(NULL)
, mNewKitButton(NULL)
, mAuditionSampleIdx(0)
, mAuditionInc(0)
, mAuditionSlider(NULL)
, mAuditionPadIdx(0)
, mNewKitNameEntry(NULL)
, mLoadingSamples(false)
, mShuffleSpeedsButton(NULL)
{
   ReadKits();
   
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
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
   mNewKitNameEntry = new TextEntry(this,"kitname",130,4,7,mNewKitName);
   mAuditionSlider = new FloatSlider(this,"aud",140,50,40,15,&mAuditionInc,-1,1,0);
   mShuffleSpeedsButton = new ClickButton(this,"shuffle",4,34);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
   {
      int x = 5 + (i % 4) * 70;
      int y = 70 + (3-(i / 4)) * 70;
      mVolSliders[i] = new FloatSlider(this,("vol"+ofToString(i)).c_str(),x+5,y+37,60,15,&mSamples[i].mVol,0,1,2);
      mSpeedSliders[i] = new FloatSlider(this,("spd"+ofToString(i)).c_str(),x+5,y+53,60,15,&mSamples[i].mSpeed,.2f,3,2);
   }
   
   for (int i=0; i<mKits.size(); ++i)
      mKitSelector->AddLabel(mKits[i].mName.c_str(), i);
}

DrumPlayer::~DrumPlayer()
{
   delete[] mWriteBuffer;
   delete[] mOutputBuffer;
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
         mSamples[i].mSample.Read(mKits[kit].mSampleFiles[i].c_str());
         mSamples[i].mLinkId = mKits[kit].mLinkIds[i];
         mSamples[i].mVol = mKits[kit].mVols[i];
         mSamples[i].mSpeed = mKits[kit].mSpeeds[i];
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
   
   if (!mEnabled || GetTarget() == NULL)
      return;
   
   ComputeSliders(0);
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   float volSq = mVolume * mVolume;
   
   Clear(mOutputBuffer, bufferSize);
   
   if (!mLoadingSamples)
   {
      mLoadSamplesMutex.lock();
      mLoadingSamples = true;
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         mSamples[i].mSample.SetRate(mSpeed * mSamples[i].mSpeed);
         if (mSamples[i].mSample.ConsumeData(mWriteBuffer, bufferSize, true))
         {
            for (int j=0; j<bufferSize; ++j)
            {
               float sample = mWriteBuffer[j] * mVelocity[i] * volSq * GetSampleVol(i);
               mOutputBuffer[j] += sample;
            }
         }
      }
      mLoadingSamples = false;
      mLoadSamplesMutex.unlock();
   }
   
   GetVizBuffer()->WriteChunk(mOutputBuffer, bufferSize);
   
   Add(out, mOutputBuffer, bufferSize);
}

void DrumPlayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   pitch %= 24;
   if (pitch >= 0 && pitch < NUM_DRUM_HITS)
   {
      if (velocity > 0)
      {
         if (mSamples[pitch].mSample.GetPlayPosition() <= 0 ||
             mSamples[pitch].mSample.GetPlayPosition() > 2000.0f / mSamples[pitch].mSample.GetSampleRateRatio())
         {
            //reset all linked drum hits
            int playingId = mSamples[pitch].mLinkId;
            for (int i=0; i<NUM_DRUM_HITS; ++i)
            {
               if (mSamples[i].mLinkId == playingId && mSamples[i].mSample.GetPlayPosition() > 100)
                  mSamples[i].mSample.Reset();
            }

            //play this one
            mSamples[pitch].mSample.Play(mSpeed * ofRandom(.99f,1.01f));
            mVelocity[pitch] = velocity / 127.0f;
            
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
               mSamples[sampleIdx].mSample.Read(files[i].c_str());
               mLoadingSamples = false;
               mLoadSamplesMutex.unlock();
               mSamples[sampleIdx].mLinkId = sLoadId;
               mSamples[sampleIdx].mVol = 1;
               mSamples[sampleIdx].mSpeed = 1;
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
         mSamples[sampleIdx].mSample.Create(sample->Data(), sample->LengthInSamples());
         mLoadingSamples = false;
         mLoadSamplesMutex.unlock();
         mSamples[sampleIdx].mLinkId = sLoadId;
         mSamples[sampleIdx].mVol = 1;
         mSamples[sampleIdx].mSpeed = 1;
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
   y /= 35;
   if (x < 4 && y/2 < 4 && y%2 == 0)
   {
      int sampleIdx = GetAssociatedSampleIndex(x, y/2);
      if (sampleIdx != -1)
      {
         mSamples[sampleIdx].mSample.Play(mSpeed);
         mVelocity[sampleIdx] = 1;
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

   if (mEditMode)
   {
      mSaveButton->Draw();
      mNewKitButton->Draw();
      mNewKitNameEntry->Draw();
      mAuditionSlider->Draw();

      ofPushMatrix();
      ofTranslate(5, 70);
      for (int i=0; i<4; ++i)
      {
         for (int j=0; j<4; ++j)
         {
            ofSetColor(200,100,0,gModuleDrawAlpha);
            int sampleIdx = GetAssociatedSampleIndex(i, j);
            Sample* sample = NULL;
            if (sampleIdx != -1)
               sample = &(mSamples[sampleIdx].mSample);
            if (sample &&
                sample->IsPlaying() &&
                sample->GetPlayPosition() < gSampleRate * .25f)
               ofFill();
            else
               ofNoFill();
            ofRect(i*70,j*70,70,70);

            ofSetColor(255,255,255,gModuleDrawAlpha);
            if (sample)
            {
               string name = sample->Name();
               for (int k=1; k<name.length(); ++k)
               {
                  if (GetStringWidth(name.substr(0,k)) > 60 &&
                      name[k-2] != '\n')
                     name = name.substr(0,k-2) + '\n' + name.substr(k-2,name.length()-1);
               }
               DrawText(GetDrumHitName(GetAssociatedSampleIndex(i, j)) + ":\n" + name,i*70+5,j*70+10);
            }
         }
      }
      ofPopMatrix();
      
      for (int i=0; i<NUM_DRUM_HITS; ++i)
      {
         mVolSliders[i]->Draw();
         mSpeedSliders[i]->Draw();
      }
   }
}

int DrumPlayer::GetAssociatedSampleIndex(int x, int y)
{
   int pos = x+(3-y)*4;
   if (pos < 16)
      return pos;
   return -1;
}

float DrumPlayer::GetSampleVol(int sampleIdx)
{
   return mSamples[sampleIdx].mVol;
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
            string path = mSamples[j].mSample.GetReadPath();
            ofStringReplace(path, File(ofToDataPath("")).getFullPathName().toStdString(), "");
            
            mKits[i].mSampleFiles[j] = path;
            mKits[i].mLinkIds[j] = mSamples[j].mLinkId;
            mKits[i].mVols[j] = mSamples[j].mVol;
            mKits[i].mSpeeds[j] = mSamples[j].mSpeed;
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
      kit.mSampleFiles[i] = mSamples[i].mSample.GetReadPath();
      kit.mLinkIds[i] = mSamples[i].mLinkId;
   }

   mKits.push_back(kit);
   mLoadedKit = mKits.size() - 1;
   mKitSelector->AddLabel(kit.mName.c_str(), mLoadedKit);
}

void DrumPlayer::ShuffleSpeeds()
{
   for (int j=0; j<NUM_DRUM_HITS; ++j)
   {
      mSamples[j].mVol *= ofRandom(.9f,1.1f);
      mSamples[j].mSpeed *= ofRandom(.9f,1.1f);
   }
}

void DrumPlayer::GetModuleDimensions(int& width, int& height)
{
   if (mEditMode)
   {
      width = 300;
      height = 370;
   }
   else
   {
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
            mSamples[mAuditionPadIdx].mSample.Read(file.c_str());
            mLoadingSamples = false;
            mLoadSamplesMutex.unlock();
            mSamples[mAuditionPadIdx].mSample.Play(mSpeed);
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
}

void DrumPlayer::ButtonClicked(ClickButton *button)
{
   if (button == mSaveButton)
      SaveKits();
   if (button == mNewKitButton)
      CreateKit();
   if (button == mShuffleSpeedsButton)
      ShuffleSpeeds();
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
      mSamples[i].mSample.SaveState(out);
      out << mSamples[i].mLinkId;
      out << mSamples[i].mVol;
      out << mSamples[i].mSpeed;
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
      mSamples[i].mSample.LoadState(in);
      in >> mSamples[i].mLinkId;
      in >> mSamples[i].mVol;
      in >> mSamples[i].mSpeed;
   }
}


