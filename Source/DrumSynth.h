//
//  DrumSynth.h
//  Bespoke
//
//  Created by Ryan Challinor on 8/5/14.
//
//

#ifndef __Bespoke__DrumSynth__
#define __Bespoke__DrumSynth__

#include <iostream>
#include "IAudioSource.h"
#include "Sample.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "Transport.h"
#include "EnvOscillator.h"
#include "ADSR.h"
#include "ADSRDisplay.h"
#include "RadioButton.h"
#include "PeakTracker.h"
#include "BiquadFilter.h"
#include "MidiDevice.h"
#include "DrumPlayer.h"
#include "TextEntry.h"

class MidiController;

#define PAD_DRAW_SIZE 140

class DrumSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IButtonListener, public IIntSliderListener, public IRadioButtonListener, public MidiDeviceListener, public ITextEntryListener
{
public:
   DrumSynth();
   ~DrumSynth();
   static IDrawableModule* Create() { return new DrumSynth(); }
   
   string GetTitleLabel() override { return "drumsynth"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   struct DrumSynthHitSerialData
   {
      DrumSynthHitSerialData();
      
      EnvOscillator mTone;
      EnvOscillator mNoise;
      ::ADSR mFreqAdsr;
      float mFreq;
      float mVol;
      float mVolNoise;
   };
   
   class DrumSynthHit
   {
   public:
      DrumSynthHit(DrumSynth* parent, int index, int x, int y);
      
      void CreateUIControls();
      void Play(double time, float velocity);
      void Process(double time, float* out, int bufferSize);
      float Level() { return mLevel.GetPeak(); }
      void Draw();
      
      DrumSynthHitSerialData mData;
      float mPhase;
      ADSRDisplay* mToneAdsrDisplay;
      ADSRDisplay* mFreqAdsrDisplay;
      ADSRDisplay* mNoiseAdsrDisplay;
      FloatSlider* mVolSlider;
      FloatSlider* mFreqSlider;
      RadioButton* mToneType;
      double mStartTime;
      PeakTracker mLevel;
      FloatSlider* mVolNoiseSlider;
      DrumSynth* mParent;
      int mIndex;
      int mX;
      int mY;
   };
   
   struct StoredDrumKit
   {
      string mName;
      DrumSynthHitSerialData mHits[NUM_DRUM_HITS];
   };
   
   void LoadKit(int kit);
   int GetAssociatedSampleIndex(int x, int y);
   void ReadKits();
   void SaveKits();
   void CreateKit();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   DrumSynthHit* mHits[NUM_DRUM_HITS];
   int mCurrentEditHit;
   float mVelocity[NUM_DRUM_HITS];
   
   float* mOutputBuffer;
   float mVolume;
   int mLoadedKit;
   FloatSlider* mVolSlider;
   DropdownList* mKitSelector;
   bool mEditMode;
   Checkbox* mEditCheckbox;
   std::vector<StoredDrumKit> mKits;
   ClickButton* mSaveButton;
   ClickButton* mNewKitButton;
   MidiController* mTwister;
   char mNewKitName[MAX_TEXTENTRY_LENGTH];
   TextEntry* mNewKitNameEntry;
};

#endif /* defined(__Bespoke__DrumSynth__) */

