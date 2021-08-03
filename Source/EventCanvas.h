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

#ifndef __Bespoke__EventCanvas__
#define __Bespoke__EventCanvas__

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
   
   string GetTitleLabel() override { return "event canvas"; }
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
   
   vector<IUIControl*> ControlsToIgnoreInSaveState() const override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   void UpdateNumColumns();
   void SyncControlCablesToCanvas();
   
   Canvas* mCanvas;
   CanvasControls* mCanvasControls;
   CanvasScrollbar* mCanvasScrollbarHorizontal;
   float mScrollPartial;
   TextEntry* mNumMeasuresEntry;
   int mNumMeasures;
   ClickButton* mQuantizeButton;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   float mPosition;
   vector<PatchCableSource*> mControlCables;
   vector<ofColor> mRowColors;
   bool mRecord;
   Checkbox* mRecordCheckbox;
   float mPreviousPosition;
   
   struct ControlConnection
   {
      IUIControl* mUIControl;
      float mLastValue;
   };
   
   const int kMaxEventRows = 256;
   vector<ControlConnection> mRowConnections;
};

#endif /* defined(__Bespoke__EventCanvas__) */
