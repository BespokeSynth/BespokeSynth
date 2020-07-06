//
//  ClipLauncher.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/17/15.
//
//

#ifndef __Bespoke__ClipLauncher__
#define __Bespoke__ClipLauncher__

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "OpenFrameworksPort.h"
#include "JumpBlender.h"

class Sample;
class Looper;

class ClipLauncher : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITimeListener, public IButtonListener
{
public:
   ClipLauncher();
   ~ClipLauncher();
   static IDrawableModule* Create() { return new ClipLauncher(); }
   
   string GetTitleLabel() override { return "clip launcher"; }
   void CreateUIControls() override;
   
   int GetRowY(int idx);
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void OnTimeEvent(double time) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void RecalcPos(double time, int idx);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   class SampleData
   {
   public:
      SampleData()
      : mSample(nullptr)
      , mNumBars(1)
      , mVolume(1)
      , mGrabCheckbox(nullptr)
      , mPlayCheckbox(nullptr)
      , mClipLauncher(nullptr)
      , mIndex(0)
      , mPlay(false)
      , mHasSample(false)
      {}
      ~SampleData();
      
      void Init(ClipLauncher* launcher, int index);
      void Draw();
      
      Sample* mSample;
      int mNumBars;
      float mVolume;
      Checkbox* mGrabCheckbox;
      Checkbox* mPlayCheckbox;
      ClipLauncher* mClipLauncher;
      int mIndex;
      bool mPlay;
      bool mHasSample;
   };
   
   Looper* mLooper;
   
   float mVolume;
   FloatSlider* mVolumeSlider;

   vector<SampleData> mSamples;
   JumpBlender mJumpBlender;
   ofMutex mSampleMutex;
};

#endif /* defined(__Bespoke__ClipLauncher__) */
