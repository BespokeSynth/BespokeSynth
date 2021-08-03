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
//  TimelineControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#ifndef __Bespoke__TimelineControl__
#define __Bespoke__TimelineControl__

#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Slider.h"

class TimelineControl : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener
{
public:
   TimelineControl();
   ~TimelineControl();
   static IDrawableModule* Create() { return new TimelineControl(); }
   
   string GetTitleLabel() override { return "timeline control"; }
   void CreateUIControls() override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   bool IsResizable() const override { return true; }
   void Resize(float width, float height) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;
   
   float GetSliderWidth() { return mWidth - 50; }
   
   float mWidth;
   float mNumMeasures;
   float mTime;
   FloatSlider* mTimeSlider;
   bool mLoop;
   Checkbox* mLoopCheckbox;
   int mLoopStart;
   IntSlider* mLoopStartSlider;
   int mLoopEnd;
   IntSlider* mLoopEndSlider;
};

#endif /* defined(__Bespoke__TimelineControl__) */
