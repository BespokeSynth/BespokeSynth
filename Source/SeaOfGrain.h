//
//  SeaOfGrain.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/8/14.
//
//

#ifndef __Bespoke__SeaOfGrain__
#define __Bespoke__SeaOfGrain__

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "INoteReceiver.h"
#include "Granulator.h"
#include "ADSR.h"

class Sample;

#define NUM_SEAOFGRAIN_VOICES 16

class SeaOfGrain : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   SeaOfGrain();
   ~SeaOfGrain();
   static IDrawableModule* Create() { return new SeaOfGrain(); }
   
   string GetTitleLabel() override { return "sea of grain"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IDrawableModule
   void FilesDropped(vector<string> files, int x, int y) override;
   void Poll() override;
   
   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   
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
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void UpdateSample();
   int GetSampleLength() const;

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(int& x, int& y) override;
   void OnClicked(int x, int y, bool right) override;
   
   struct GrainVoice
   {
      GrainVoice();
      void Process(float* out, int outLength, float* sample, int sampleLength, int keyOffset, const vector<float>& slices);
      void Draw(float w, float h, float offset, float length, int numPitches);
      
      float mPlay;
      float mPitch;
      ModulationChain* mPitchBend;
      ModulationChain* mPressure;
      ModulationChain* mModWheel;
      
      float mGain;
      
      ADSR mADSR;
      Granulator mGranulator;
   };
   
   GrainVoice mVoices[NUM_SEAOFGRAIN_VOICES];
   
   Sample* mSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
   bool mLoading;
   IntSlider* mKeyOffsetSlider;
   int mKeyOffset;
   IntSlider* mDisplayKeysSlider;
   int mDisplayKeys;
   DropdownList* mKeyboardBaseNoteSelector;
   int mKeyboardBaseNote;
   vector<float> mSlices;
};

#endif /* defined(__Bespoke__SeaOfGrain__) */

