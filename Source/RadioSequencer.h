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
/*
  ==============================================================================

    RadioSequencer.h
    Created: 10 Jun 2017 4:53:13pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "UIGrid.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "GridController.h"
#include "Slider.h"
#include "IPulseReceiver.h"
#include "INoteReceiver.h"
#include "IDrivableSequencer.h"

class PatchCableSource;

class RadioSequencer : public IDrawableModule, public ITimeListener, public IDropdownListener, public UIGridListener, public IGridControllerListener, public IIntSliderListener, public IPulseReceiver, public INoteReceiver, public IDrivableSequencer
{
public:
   RadioSequencer();
   ~RadioSequencer();
   static IDrawableModule* Create() { return new RadioSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   //IGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }
   bool LoadOldControl(FileStreamIn& in, std::string& oldName) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void Step(double time, int pulseFlags);
   void SetGridSize(float w, float h);
   void SyncControlCablesToGrid();
   void UpdateGridLights();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   UIGrid* mGrid{ nullptr };
   NoteInterval mInterval{ kInterval_1n };
   DropdownList* mIntervalSelector{ nullptr };
   int mLength{ 4 };
   IntSlider* mLengthSlider{ nullptr };
   std::string mOldLengthStr;
   std::vector<PatchCableSource*> mControlCables;
   GridControlTarget* mGridControlTarget{ nullptr };
   bool mHasExternalPulseSource{ false };
   int mStep{ 0 };
   int mLoadRev{ -1 };
   bool mDisableAllWhenDisabled{ true };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
