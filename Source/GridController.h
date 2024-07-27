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
//  GridController.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#pragma once

#include "IUIControl.h"
#include "MidiController.h"

#define MAX_GRIDCONTROLLER_ROWS 512
#define MAX_GRIDCONTROLLER_COLS 512

class IGridController;
class PatchCableSource;

enum GridColor
{
   kGridColorOff,
   kGridColor1Dim,
   kGridColor1Bright,
   kGridColor2Dim,
   kGridColor2Bright,
   kGridColor3Dim,
   kGridColor3Bright
};

class IGridControllerListener
{
public:
   virtual ~IGridControllerListener() {}
   virtual void OnControllerPageSelected() = 0;
   virtual void OnGridButton(int x, int y, float velocity, IGridController* grid) = 0;
};

class IGridController
{
public:
   virtual ~IGridController() {}
   virtual void SetGridControllerOwner(IGridControllerListener* owner) = 0;
   virtual void SetLight(int x, int y, GridColor color, bool force = false) = 0;
   virtual void SetLightDirect(int x, int y, int color, bool force = false) = 0;
   virtual void ResetLights() = 0;
   virtual int NumCols() = 0;
   virtual int NumRows() = 0;
   virtual bool HasInput() const = 0;
   virtual bool IsMultisliderGrid() const { return false; }
   virtual bool IsConnected() const = 0;
};

class GridControlTarget : public IUIControl
{
public:
   GridControlTarget(IGridControllerListener* owner, const char* name, int x, int y);
   virtual ~GridControlTarget() {}

   void Render() override;
   static void DrawGridIcon(float x, float y);

   void SetGridController(IGridController* gridController)
   {
      mGridController = gridController;
      gridController->SetGridControllerOwner(mOwner);
   }
   IGridController* GetGridController() { return mGridController; }

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override {}
   void SetValue(float value, double time, bool forceUpdate = false) override {}
   bool CanBeTargetedBy(PatchCableSource* source) const override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool GetNoHover() const override { return true; }

private:
   void GetDimensions(float& width, float& height) override
   {
      width = 30;
      height = 15;
   }
   bool MouseMoved(float x, float y) override;

   IGridControllerListener* mOwner{ nullptr };
   IGridController* mGridController{ nullptr };
};

class GridControllerMidi : public IGridController
{
public:
   GridControllerMidi();
   virtual ~GridControllerMidi() {}

   void SetUp(GridLayout* layout, int page, MidiController* controller);
   void UnhookController();

   //IGridController
   void SetGridControllerOwner(IGridControllerListener* owner) override { mOwner = owner; }
   void SetLight(int x, int y, GridColor color, bool force = false) override;
   void SetLightDirect(int x, int y, int color, bool force = false) override;
   void ResetLights() override;
   int NumCols() override { return mCols; }
   int NumRows() override { return mRows; }
   bool HasInput() const override;
   //bool IsMultisliderGrid() const override { return mColors.empty(); }   //commented out... don't remember what types of grids this is supposed to be for
   bool IsConnected() const override { return mMidiController != nullptr; }

   void OnControllerPageSelected();
   void OnInput(int control, float velocity);

private:
   unsigned int mRows{ 8 };
   unsigned int mCols{ 8 };
   int mControls[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS]{};
   float mInput[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS]{};
   int mLights[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS]{};
   std::vector<int> mColors;
   MidiMessageType mMessageType{ MidiMessageType::kMidiMessage_Note };
   MidiController* mMidiController{ nullptr };
   int mControllerPage{ 0 };
   IGridControllerListener* mOwner{ nullptr };
};
