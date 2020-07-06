//
//  WaveformViewer.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/19/12.
//
//

#ifndef __modularSynth__WaveformViewer__
#define __modularSynth__WaveformViewer__

#include <iostream>
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"

#define BUFFER_VIZ_SIZE 2048

class WaveformViewer : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   WaveformViewer();
   virtual ~WaveformViewer();
   static IDrawableModule* Create() { return new WaveformViewer(); }
   
   string GetTitleLabel() override { return "waveform"; }
   void CreateUIControls() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   float mAudioView[BUFFER_VIZ_SIZE][2];
   bool mDoubleBufferFlip;

   int mBufferVizOffset[2];
   float mVizPhase[2];

   bool mPhaseAlign;
   float mWidth;
   float mHeight;
   bool mDrawWaveform;
   bool mDrawCircle;

   FloatSlider* mHueNote;
   FloatSlider* mHueAudio;
   FloatSlider* mHueInstrument;
   FloatSlider* mHueNoteSource;
   FloatSlider* mSaturation;
   FloatSlider* mBrightness;
};

#endif /* defined(__modularSynth__WaveformViewer__) */

