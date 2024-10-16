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
//  EventCanvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#pragma once

#include "Transport.h"
#include "Checkbox.h"
#include "Canvas.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "TextEntry.h"

class CanvasControls;
class PatchCableSource;
class CanvasScrollbar;

class EventCanvas : public IDrawableModule, public ICanvasListener, public IFloatSliderListener, public IAudioPoller, public IIntSliderListener, public IButtonListener, public IDropdownListener, public ITextEntryListener
{
public:
   EventCanvas();
   ~EventCanvas();
   static IDrawableModule* Create() { return new EventCanvas(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   IUIControl* GetUIControlForRow(int row);
   ofColor GetRowColor(int row) const;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   void OnTransportAdvanced(float amount) override;

   void CanvasUpdated(Canvas* canvas) override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   std::vector<IUIControl*> ControlsToIgnoreInSaveState() const override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void UpdateNumColumns();
   void SyncControlCablesToCanvas();
   double GetTriggerTime(double lookaheadTime, double lookaheadPos, float eventPos);

   Canvas* mCanvas{ nullptr };
   CanvasControls* mCanvasControls{ nullptr };
   CanvasScrollbar* mCanvasScrollbarHorizontal{ nullptr };
   float mScrollPartial{ 0 };
   TextEntry* mNumMeasuresEntry{ nullptr };
   int mNumMeasures{ 1 };
   ClickButton* mQuantizeButton{ nullptr };
   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };
   float mPosition{ 0 };
   std::vector<PatchCableSource*> mControlCables{};
   std::vector<ofColor> mRowColors{};
   bool mRecord{ false };
   Checkbox* mRecordCheckbox{ nullptr };
   double mPreviousPosition{ 0 };

   struct ControlConnection
   {
      IUIControl* mUIControl{ nullptr };
      float mLastValue{ 0 };
   };

   const int kMaxEventRows = 256;
   std::vector<ControlConnection> mRowConnections;
};
