//
//  OneShotLauncher.h
//  Bespoke
//
//  Created by Ryan Challinor on 10/16/14.
//
//

#ifndef __Bespoke__OneShotLauncher__
#define __Bespoke__OneShotLauncher__

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

class SampleBank;
class Sample;

struct OneShot
{
   ClickButton* mPlay;
   float mVolume;
   FloatSlider* mVolumeSlider;
};

class OneShotLauncher : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITimeListener, public IButtonListener
{
public:
   OneShotLauncher();
   ~OneShotLauncher();
   static IDrawableModule* Create() { return new OneShotLauncher(); }
   
   string GetTitleLabel() override { return "oneshot"; }
   void CreateUIControls() override;
   
   void SetSampleBank(SampleBank* bank);
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   void RecalcPos(int idx);
   void UpdateSampleList();
   int GetRowY(int idx);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   SampleBank* mBank;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
   int mSampleIndex;
   DropdownList* mSampleList;
   ClickButton* mAddBeatButton;
   int mNumActive;
   vector<OneShot> mOneShots;
};

#endif /* defined(__Bespoke__OneShotLauncher__) */

