/*
  ==============================================================================

    NoteHumanizer.h
    Created: 2 Nov 2016 7:56:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#ifndef NOTEHUMANIZER_H_INCLUDED
#define NOTEHUMANIZER_H_INCLUDED


#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"
#include "Transport.h"

class NoteHumanizer : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   NoteHumanizer();
   ~NoteHumanizer();
   static IDrawableModule* Create() { return new NoteHumanizer(); }
   
   string GetTitleLabel() override { return "note humanizer"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   struct NoteInfo
   {
      int mPitch;
      int mVelocity;
      double mTriggerTime;
      ModulationParameters mModulation;
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 40; }
   bool Enabled() const override { return mEnabled; }
   
   float mTime;
   FloatSlider* mTimeSlider;
   float mVelocity;
   FloatSlider* mVelocitySlider;
   
   ofMutex mNoteMutex;
   vector<NoteInfo> mInputNotes;
};


#endif  // NOTEHUMANIZER_H_INCLUDED
