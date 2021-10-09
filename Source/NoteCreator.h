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
//  NoteCreator.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#ifndef __Bespoke__NoteCreator__
#define __Bespoke__NoteCreator__

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "Slider.h"
#include "Transport.h"
#include "IPulseReceiver.h"

class NoteCreator : public IDrawableModule, public INoteSource, public IButtonListener, public ITextEntryListener, public IFloatSliderListener, public IAudioPoller, public IPulseReceiver
{
public:
   NoteCreator();
   virtual ~NoteCreator();
   static IDrawableModule* Create() { return new NoteCreator(); }
   
   
   void CreateUIControls() override;
   void Init() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void OnTransportAdvanced(float amount) override;
   
   void OnPulse(double time, float velocity, int flags) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void TextEntryComplete(TextEntry* entry) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
protected:
   void TriggerNote(double time, float velocity);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override { w=110; h=58; }
   
   ClickButton* mTriggerButton;
   TextEntry* mPitchEntry;
   FloatSlider* mVelocitySlider;
   FloatSlider* mDurationSlider;
   Checkbox* mNoteOnCheckbox;
   int mPitch;
   float mVelocity;
   float mDuration;
   double mStartTime;
   bool mNoteOn;
   bool mNoteOnByTrigger;
   int mVoiceIndex;
};

#endif /* defined(__Bespoke__NoteCreator__) */
