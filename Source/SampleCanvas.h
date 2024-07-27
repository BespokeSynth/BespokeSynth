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
//  SampleCanvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/24/15.
//
//

#pragma once

#include "IDrawableModule.h"
#include "IAudioSource.h"
#include "Transport.h"
#include "Canvas.h"
#include "Slider.h"
#include "DropdownList.h"

class CanvasControls;
class CanvasTimeline;
class CanvasScrollbar;

class SampleCanvas : public IDrawableModule, public IAudioSource, public ICanvasListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   SampleCanvas();
   ~SampleCanvas();
   static IDrawableModule* Create() { return new SampleCanvas(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   void Process(double time) override;
   NoteInterval GetInterval() const { return mInterval; }

   void CanvasUpdated(Canvas* canvas) override;

   void OnClicked(float x, float y, bool right) override;

   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void SetNumMeasures(int numMeasures);
   void UpdateNumColumns();
   double GetCurPos(double time) const;

   Canvas* mCanvas{ nullptr };
   CanvasControls* mCanvasControls{ nullptr };
   CanvasTimeline* mCanvasTimeline{ nullptr };
   CanvasScrollbar* mCanvasScrollbarHorizontal{ nullptr };
   CanvasScrollbar* mCanvasScrollbarVertical{ nullptr };

   IntSlider* mNumMeasuresSlider{ nullptr };
   int mNumMeasures{ 4 };
   NoteInterval mInterval{ NoteInterval::kInterval_1n };
   DropdownList* mIntervalSelector{ nullptr };
};
