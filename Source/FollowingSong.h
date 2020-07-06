//
//  FollowingSong.h
//  Bespoke
//
//  Created by Ryan Challinor on 10/15/14.
//
//

#ifndef __Bespoke__FollowingSong__
#define __Bespoke__FollowingSong__

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "RadioButton.h"
#include "Slider.h"
#include "IAudioSource.h"
#include "MidiReader.h"
#include "Sample.h"
#include "ofxJSONElement.h"

class FollowingSong : public IDrawableModule, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public IIntSliderListener, public IAudioSource, public IFloatSliderListener
{
public:
   FollowingSong();
   ~FollowingSong();
   static IDrawableModule* Create() { return new FollowingSong(); }
   
   string GetTitleLabel() override { return "following song"; }
   void CreateUIControls() override;
   
   void LoadSample(const char* file);
   void SetPlaybackInfo(bool play, int position, float speed, float volume);
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override { width=560; height=130; }
   
   ofMutex mLoadSongMutex;
   bool mLoadingSong;
   
   Sample mSample;
   float mVolume;
   bool mPlay;
   bool mMute;
   Checkbox* mMuteCheckbox;
};


#endif /* defined(__Bespoke__FollowingSong__) */

