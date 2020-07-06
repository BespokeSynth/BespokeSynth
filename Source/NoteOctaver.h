//
//  NoteOctaver.h
//  modularSynth
//
//  Created by Ryan Challinor on 5/27/13.
//
//

#ifndef __modularSynth__NoteOctaver__
#define __modularSynth__NoteOctaver__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class NoteOctaver : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   NoteOctaver();
   static IDrawableModule* Create() { return new NoteOctaver(); }
   
   string GetTitleLabel() override { return "octaver"; }
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
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 22; }
   bool Enabled() const override { return mEnabled; }

   
   
   int mOctave;
   IntSlider* mOctaveSlider;
   std::list<NoteInfo> mInputNotes;
   ofMutex mNotesMutex;
};


#endif /* defined(__modularSynth__NoteOctaver__) */

