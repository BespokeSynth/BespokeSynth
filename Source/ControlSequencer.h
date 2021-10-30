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
//  ControlSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 8/27/15.
//
//

#ifndef __Bespoke__ControlSequencer__
#define __Bespoke__ControlSequencer__

#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "Slider.h"
#include "IPulseReceiver.h"
#include "INoteReceiver.h"
#include "IDrivableSequencer.h"

class PatchCableSource;

class ControlSequencer : public IDrawableModule, public ITimeListener, public IDropdownListener, public UIGridListener, public IButtonListener, public IIntSliderListener, public IPulseReceiver, public INoteReceiver, public IDrivableSequencer
{
public:
   ControlSequencer();
   ~ControlSequencer();
   static IDrawableModule* Create() { return new ControlSequencer(); }


   void CreateUIControls() override;
   void Init() override;

   IUIControl* GetUIControl() const { return mUIControl; }

   //IGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   //IDrawableModule
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }
   
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   bool LoadOldControl(FileStreamIn& in, std::string& oldName) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   static std::list<ControlSequencer*> sControlSequencers;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   void Step(double time, int pulseFlags);
   void SetGridSize(float w, float h);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   UIGrid* mGrid{ nullptr };
   IUIControl* mUIControl{ nullptr };
   NoteInterval mInterval{ kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };
   int mLength{ 16 };
   IntSlider* mLengthSlider{ nullptr };
   std::string mOldLengthStr;
   int mLoadRev{ -1 };
   PatchCableSource* mControlCable{ nullptr };
   ClickButton* mRandomize{ nullptr };
   bool mHasExternalPulseSource{ false };
   int mStep{ 0 };

   TransportListenerInfo* mTransportListenerInfo;
};

#endif /* defined(__Bespoke__ControlSequencer__) */
