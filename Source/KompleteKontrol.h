/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  KompleteKontrol.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/11/15.
//
//

#ifndef __Bespoke__KompleteKontrol__
#define __Bespoke__KompleteKontrol__

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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Poll() override;
   void Exit() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //IScaleListener
   void OnScaleChanged() override;

   //IKontrolListener
   void OnKontrolButton(int control, bool on) override;
   void OnKontrolEncoder(int control, float change) override;
   void OnKontrolOctave(int octave) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   const int kNumKeys = 61;

   bool IsEnabled() const override { return true; }

private:
   struct TextBox
   {
      TextBox()
      : slider(false)
      , amount(0)
      {}
      bool slider;
      float amount;
      std::string line1;
      std::string line2;
   };

   void UpdateKeys();
   void UpdateText();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 100;
      height = 18;
   }

   KontrolKommunicator mKontrol;
   bool mInitialized;
   int mKeyOffset;
   bool mNeedKeysUpdate;
   std::string mCurrentText;
   uint16_t mCurrentSliders[NUM_SLIDER_SEGMENTS];
   TextBox mTextBoxes[9];
   MidiController* mController;

   PatchCableSource* mMidiControllerCable;
};

#endif /* defined(__Bespoke__KompleteKontrol__) */
