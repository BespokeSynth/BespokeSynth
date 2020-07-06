//
//  ControllingSong.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/29/14.
//
//

#ifndef __Bespoke__ControllingSong__
#define __Bespoke__ControllingSong__

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

class FollowingSong;

class ControllingSong : public IDrawableModule, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public IIntSliderListener, public IAudioSource, public IFloatSliderListener
{
public:
   ControllingSong();
   ~ControllingSong();
   static IDrawableModule* Create() { return new ControllingSong(); }
   
   string GetTitleLabel() override { return "controlling song"; }
   void CreateUIControls() override;
   
   void Init() override;
   void Poll() override;
   
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
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override { width=560; height=160; }
   
   void LoadSong(int index);
   
   ofMutex mLoadSongMutex;
   bool mLoadingSong;
   
   int mCurrentSongIndex;
   MidiReader mMidiReader;
   Sample mSample;
   float mVolume;
   FloatSlider* mVolumeSlider;
   bool mNeedNewSong;
   double mSongStartTime;
   ofxJSONElement mSongList;
   int mTestBeatOffset;
   IntSlider* mTestBeatOffsetSlider;
   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mShuffle;
   Checkbox* mShuffleCheckbox;
   ClickButton* mPhraseForwardButton;
   ClickButton* mPhraseBackButton;
   float mSpeed;
   FloatSlider* mSpeedSlider;
   bool mMute;
   Checkbox* mMuteCheckbox;
   
   DropdownList* mSongSelector;
   ClickButton* mNextSongButton;
   int mShuffleIndex;
   vector<int> mShuffleList;
   vector<FollowingSong*> mFollowSongs;
};

#endif /* defined(__Bespoke__ControllingSong__) */

