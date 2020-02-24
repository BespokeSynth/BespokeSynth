//
//  SampleBank.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#include "SampleBank.h"
#include <fstream>
#include "SynthGlobals.h"
#include "ModularSynth.h"

SampleBank::SampleBank()
: mSamplesDropdown(nullptr)
, mSampleIdx(-1)
{
}

SampleBank::~SampleBank()
{
   for (int i=0; i<mSamples.size(); ++i)
      delete mSamples[i].mSample;
}

void SampleBank::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mSamplesDropdown = new DropdownList(this,"sample",4,4,&mSampleIdx);
}

void SampleBank::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mSamplesDropdown->Draw();
   
   if (mSampleIdx >= 0 && mSampleIdx < mSamples.size())
   {
      ofPushMatrix();
      ofTranslate(5, 22);
      DrawAudioBuffer(190, 55, mSamples[mSampleIdx].mSample->Data(), 0, mSamples[mSampleIdx].mSample->LengthInSamples(), -1);
      ofPopMatrix();
   }
}

void SampleBank::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   if (y > 22)
   {
      if (mSampleIdx >= 0 && mSampleIdx < mSamples.size())
      {
         TheSynth->GrabSample(mSamples[mSampleIdx].mSample->Data(), false, mSamples[mSampleIdx].mSample->GetNumBars());
      }
   }
}

bool SampleSorter(const SampleInfo& info1, const SampleInfo& info2)
{
   if (info1.mType == info2.mType)
      return string(info1.mSample->Name())+"1" < string(info2.mSample->Name());
   return info1.mType < info2.mType;
}

void SampleBank::LoadList(const char* filename)
{
   for (int i=0; i<mSamples.size(); ++i)
      delete mSamples[i].mSample;
   mSamples.clear();
   mSamplesDropdown->Clear();
   
   ifstream fin(ofToDataPath(filename).c_str());

   string line;
   if (fin.is_open())
   {
      while ( fin.good() )
      {
         getline (fin,line);
         vector<string> quotes = ofSplitString(line,"\"");
         if (quotes.size() == 3)
         {
            string wavFile = quotes[1];
            vector<string> tokens = ofSplitString(quotes[2]," ");
            int numBars = atoi(tokens[1].c_str());
            float offset = atof(tokens[2].c_str());
            float vol = atoi(tokens[3].c_str());
            string type = tokens[4];

            Sample* sample = new Sample();
            sample->Read(wavFile.c_str());

            SampleInfo info;
            info.mSample = sample;
            sample->SetNumBars(numBars);
            info.mOffset = offset;
            info.mVol = vol;
            info.mType = type;

            mSamples.push_back(info);
            mSamplesDropdown->AddLabel(sample->Name(), (int)mSamples.size()-1);
         }
      }
      fin.close();
   }
   
   //sort(mSamples.begin(), mSamples.end(), SampleSorter);
   
   for (ISampleBankListener* listener : mListeners)
      listener->OnSamplesLoaded(this);
}

const SampleInfo& SampleBank::GetSampleInfo(int index)
{
   assert(index >= 0);
   assert(index < mSamples.size());
   return mSamples[index];
}

void SampleBank::AddListener(ISampleBankListener* listener)
{
   mListeners.push_back(listener);
   if (!mSamples.empty()) //already loaded
      listener->OnSamplesLoaded(this);
}

void SampleBank::RemoveListener(ISampleBankListener* listener)
{
   mListeners.remove(listener);
}

void SampleBank::FilesDropped(vector<string> files, int x, int y)
{
   Sample* sample = new Sample();
   sample->Read(files[0].c_str());

   SampleInfo info;
   info.mSample = sample;
   sample->SetNumBars(int(sample->LengthInSamples() / gSampleRate * .5f));
   info.mOffset = 0;
   info.mVol = 1;
   info.mType = "sample";

   mSamples.push_back(info);
}

namespace {
   void FillSampleList(DropdownList* list)
   {
      assert(list);
      vector<string> lists;
      DirectoryIterator dir(File(ofToDataPath("samples")), false);
      while(dir.next())
      {
         File file = dir.getFile();
         if (file.getFileExtension() ==  ".txt")
            lists.push_back("samples/"+file.getFileName().toStdString());
      }
      for (int i=0; i<lists.size(); ++i)
         list->AddLabel(lists[i].c_str(), i);
   }
}

void SampleBank::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("samplelist", moduleInfo, "", FillSampleList);

   SetUpFromSaveData();
}

void SampleBank::SetUpFromSaveData()
{
   LoadList(mModuleSaveData.GetString("samplelist").c_str());
}

