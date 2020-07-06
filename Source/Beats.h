//
//  Beats.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/2/14.
//
//

#ifndef __modularSynth__Beats__
#define __modularSynth__Beats__

#include <iostream>
#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "RadioButton.h"
#include "OpenFrameworksPort.h"
#include "SampleBank.h"
#include "BiquadFilter.h"
#include "Ramp.h"

class Beats;
struct SampleInfo;

#define BEAT_COLUMN_WIDTH 110

struct BeatData
{
   BeatData() : mBeat(nullptr) {}
   void LoadBeat(const SampleInfo* info);
   void RecalcPos(double time, bool doubleTime);
   
   int mNumBars;
   Sample* mBeat;
};

class BeatColumn
{
public:
   BeatColumn(Beats* owner, int index);
   ~BeatColumn();
   void Draw(int x, int y);
   void CreateUIControls();
   void AddBeat(Sample* sample);
   void Process(double time, float* buffer, int bufferSize);
   
   void RadioButtonUpdated(RadioButton* list, int oldVal);
   
private:
   RadioButton* mSelector;
   int mSampleIndex;
   float mVolume;
   FloatSlider* mVolumeSlider;
   BeatData mBeatData;
   int mIndex;
   float mFilter;
   FloatSlider* mFilterSlider;
   BiquadFilter mLowpass;
   BiquadFilter mHighpass;
   Beats* mOwner;
   Ramp mFilterRamp;
   bool mDoubleTime;
   Checkbox* mDoubleTimeCheckbox;
};

class Beats : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITimeListener, public IButtonListener, public IRadioButtonListener, public ISampleBankListener
{
public:
   Beats();
   ~Beats();
   static IDrawableModule* Create() { return new Beats(); }
   
   string GetTitleLabel() override { return "beats"; }
   void CreateUIControls() override;
   
   void Init() override;
   
   const SampleInfo* GetSampleInfo(int columnIdx, int sampleIdx);
   
   //SampleBankListener
   void OnSamplesLoaded(SampleBank* bank) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   SampleBank* mBank;
   
   float* mWriteBuffer;
   vector<BeatColumn*> mBeatColumns;
   int mRows;
   PatchCableSource* mSampleBankCable;
};

#endif /* defined(__modularSynth__Beats__) */

