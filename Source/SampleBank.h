//
//  SampleBank.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/13.
//
//

#ifndef __modularSynth__SampleBank__
#define __modularSynth__SampleBank__

#include <iostream>
#include "IDrawableModule.h"
#include "Sample.h"
#include "DropdownList.h"

class SampleBank;

class ISampleBankListener
{
public:
   virtual ~ISampleBankListener() {}
   virtual void OnSamplesLoaded(SampleBank* bank) = 0;
};

struct SampleInfo
{
   Sample* mSample;
   float mOffset;
   float mVol;
   string mType;
};

class SampleBank : public IDrawableModule, public IDropdownListener
{
public:
   SampleBank();
   ~SampleBank();
   static IDrawableModule* Create() { return new SampleBank(); }
   
   string GetTitleLabel() override { return "sample bank"; }
   
   void CreateUIControls() override;

   void FilesDropped(vector<string> files, int x, int y) override;

   void LoadList(const char* filename);
   const vector<SampleInfo>& GetSamples() { return mSamples; }
   const SampleInfo& GetSampleInfo(int index);
   void AddListener(ISampleBankListener* listener);
   void RemoveListener(ISampleBankListener* listener);
   
   void DropdownUpdated(DropdownList* list, int oldVal) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=200; height=80; }
   void OnClicked(int x, int y, bool right) override;
   
   vector<SampleInfo> mSamples;
   list<ISampleBankListener*> mListeners;
   
   DropdownList* mSamplesDropdown;
   int mSampleIdx;
};

#endif /* defined(__modularSynth__SampleBank__) */
