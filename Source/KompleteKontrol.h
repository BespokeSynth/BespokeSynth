//
//  KompleteKontrol.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/11/15.
//
//

#ifndef __Bespoke__KompleteKontrol__
#define __Bespoke__KompleteKontrol__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Checkbox.h"
#include "Stutter.h"
#include "CFMessaging/KontrolKommunicator.h"
#include "NoteEffectBase.h"
#include "Scale.h"

class MidiController;

#define NUM_SLIDER_SEGMENTS 72

class KompleteKontrol : public IDrawableModule, public NoteEffectBase, public IScaleListener, public IKontrolListener
{
public:
   KompleteKontrol();
   ~KompleteKontrol();
   static IDrawableModule* Create() { return new KompleteKontrol(); }
   
   string GetTitleLabel() override { return "komplete kontrol"; }
   void CreateUIControls() override;
   
   void Poll() override;
   void Exit() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   //IScaleListener
   void OnScaleChanged() override;
   
   //IKontrolListener
   void OnKontrolButton(int control, bool on) override;
   void OnKontrolEncoder(int control, float change) override;
   void OnKontrolOctave(int octave) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   const int kNumKeys = 61;
   
private:
   struct TextBox
   {
      TextBox() : slider(false), amount(0) {}
      bool slider;
      float amount;
      string line1;
      string line2;
   };
   
   void UpdateKeys();
   void UpdateText();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width=100; height=18; }
   
   KontrolKommunicator mKontrol;
   bool mInitialized;
   int mKeyOffset;
   bool mNeedKeysUpdate;
   string mCurrentText;
   uint16_t mCurrentSliders[NUM_SLIDER_SEGMENTS];
   TextBox mTextBoxes[9];
   MidiController* mController;
   
   PatchCableSource* mMidiControllerCable;
};

#endif /* defined(__Bespoke__KompleteKontrol__) */
