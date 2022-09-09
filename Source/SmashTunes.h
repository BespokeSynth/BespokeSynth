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
//  SmashTunes.h
//  Bespoke
//
//  Created by Grace Lovelace on 9/9/2022.
//
//

#ifndef __Bespoke__SmashTunes__
#define __Bespoke__SmashTunes__

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"
#include "Slider.h"
#include "IPulseReceiver.h"

class SmashTunes : public IDrawableModule, public INoteSource, public ITextEntryListener, public IFloatSliderListener, public IIntSliderListener, public IPulseReceiver
{
public:
   SmashTunes();
   virtual ~SmashTunes();
   static IDrawableModule* Create() { return new SmashTunes(); }


   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void OnPulse(double time, float velocity, int flags) override;

   void TextEntryComplete(TextEntry* entry) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

protected:
   void TriggerNote(double time, float velocity);

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override
   {
      w = mWidth;
      h = mHeight;
   }

   int mWidth{ 300 };
   int mHeight{ 20 };

   TextEntry* mTextToSmashEntry{ nullptr };
   FloatSlider* mVelocitySlider{ nullptr };
   FloatSlider* mDurationSlider{ nullptr };
   IntSlider* mMinNoteSlider{ nullptr };
   IntSlider* mMaxNoteSlider{ nullptr };
   char mTextToSmash[MAX_TEXTENTRY_LENGTH]{ "Wow! Cool princess, IMO." };
   float mVelocity{ 1 };
   float mDuration{ 100 };
   int mMinNote{ 0 };
   int mMaxNote{ 93 };
   double mStartTime{ 0 };
   int mVoiceIndex{ -1 };

   private:
   int mCurrentStringIndex{ 0 };
   int NextNote();
};

#endif /* defined(__Bespoke__SmashTunes__) */
