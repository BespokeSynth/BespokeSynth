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

    GridSliders.h
    Created: 2 Aug 2021 10:32:04pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "GridController.h"

class PatchCableSource;

class GridSliders : public IDrawableModule, public IDropdownListener, public IGridControllerListener
{
public:
   GridSliders();
   ~GridSliders();
   static IDrawableModule* Create() { return new GridSliders(); }


   void CreateUIControls() override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   void DropdownUpdated(DropdownList* list, int oldVal) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   enum class Direction
   {
      kHorizontal,
      kVertical
   };

   Direction mDirection{ Direction::kVertical };
   DropdownList* mDirectionSelector{ nullptr };
   std::array<PatchCableSource*, 32> mControlCables{};
   GridControlTarget* mGridControlTarget{ nullptr };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
