//
//  Sampler.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#ifndef __modularSynth__Sampler__
#define __modularSynth__Sampler__

#include <iostream>
#include "IAudioProcessor.h"
#include "PolyphonyMgr.h"
#include "SampleVoice.h"
#include "ADSR.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"
#include "Checkbox.h"
#include "PitchDetector.h"

class ofxJSONElement;

#define MAX_SAMPLER_LENGTH 2*gSampleRate

class Sampler : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener
{
public:
   Sampler();
   ~Sampler();
   static IDrawableModule* Create() { return new Sampler(); }
   
   string GetTitleLabel() override { return "sampler"; }
   void CreateUIControls() override;
   
   void Poll() override;
   
   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void StopRecording();
   float DetectSampleFrequency();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   
   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   SampleVoiceParams mVoiceParams;
   FloatSlider* mVolSlider;
   ADSRDisplay* mADSRDisplay;
   float mThresh;
   FloatSlider* mThreshSlider;

   float* mSampleData;
   int mRecordPos;
   bool mRecording;
   Checkbox* mRecordCheckbox;
   bool mPitchCorrect;
   Checkbox* mPitchCorrectCheckbox;
   bool mPassthrough;
   Checkbox* mPassthroughCheckbox;
   
   ChannelBuffer mWriteBuffer;
   
   PitchDetector mPitchDetector;
   bool mWantDetectPitch;
};


#endif /* defined(__modularSynth__Sampler__) */

