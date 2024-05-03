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

class ControlSequencer : public IDrawableModule, public ITimeListener, public IDropdownListener, public UIGridListener, public IButtonListener, public IIntSliderListener, public IPulseReceiver, public INoteReceiver, public IDrivableSequencer, public IFloatSliderListener
{
public:
   ControlSequencer();
   ~ControlSequencer();
   static IDrawableModule* Create() { return new ControlSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   IUIControl* GetUIControl() const { return mTargets.size() == 0 ? nullptr : mTargets[0]; }

   //IGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   //IDrawableModule
   void Poll() override;
   bool IsResizable() const override { return !mSliderMode; }
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

   void CheckboxUpdated(Checkbox* checkbox, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   bool LoadOldControl(FileStreamIn& in, std::string& oldName) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

   static std::list<ControlSequencer*> sControlSequencers;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void Step(double time, int pulseFlags);
   void SetGridSize(float w, float h);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   UIGrid* mGrid{ nullptr };
   std::array<IUIControl*, IDrawableModule::kMaxOutputsPerPatchCableSource> mTargets{};
   NoteInterval mInterval{ kInterval_4n };
   DropdownList* mIntervalSelector{ nullptr };
   int mLength{ 8 };
   IntSlider* mLengthSlider{ nullptr };
   std::string mOldLengthStr;
   int mLoadRev{ -1 };
   PatchCableSource* mControlCable{ nullptr };
   ClickButton* mRandomize{ nullptr };
   bool mHasExternalPulseSource{ false };
   int mStep{ 0 };
   bool mSliderMode{ true };
   std::array<FloatSlider*, 32> mStepSliders{};
   bool mRecord{ false };
   Checkbox* mRecordCheckbox{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};

#endif /* defined(__Bespoke__ControlSequencer__) */
