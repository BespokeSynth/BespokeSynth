//
//  KarplusStrong.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#ifndef __modularSynth__KarplusStrong__
#define __modularSynth__KarplusStrong__

#include <iostream>
#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "KarplusStrongVoice.h"
#include "ADSR.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "BiquadFilterEffect.h"

class KarplusStrong : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener
{
public:
   KarplusStrong();
   ~KarplusStrong();
   static IDrawableModule* Create() { return new KarplusStrong(); }
   
   string GetTitleLabel() override { return "karplus/strong"; }
   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 250; height = 90; }
   bool Enabled() const override { return mEnabled; }

   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   KarplusStrongVoiceParams mVoiceParams;
   FloatSlider* mFilterSlider;
   FloatSlider* mFeedbackSlider;
   FloatSlider* mVolSlider;
   DropdownList* mSourceDropdown;
   Checkbox* mMuteCheckbox;
   BiquadFilterEffect mBiquad;
   
   Checkbox* mStretchCheckbox;
   FloatSlider* mExciterFreqSlider;
   FloatSlider* mExciterAttackSlider;
   FloatSlider* mExciterDecaySlider;

   ChannelBuffer mWriteBuffer;
};


#endif /* defined(__modularSynth__KarplusStrong__) */

