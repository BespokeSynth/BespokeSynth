//
//  Capo.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/5/14.
//
//

#ifndef __modularSynth__Capo__
#define __modularSynth__Capo__

#include <iostream>

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class Capo : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   Capo();
   static IDrawableModule* Create() { return new Capo(); }
   
   string GetTitleLabel() override { return "capo"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   struct NoteInfo
   {
      int mPitch;
      int mVelocity;
      int mVoiceIdx;
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   int mCapo;
   IntSlider* mCapoSlider;
   std::list<NoteInfo> mInputNotes;
   ofMutex mNotesMutex;
};


#endif /* defined(__modularSynth__Capo__) */

