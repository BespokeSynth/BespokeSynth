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
//  ComboGridController.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/10/15.
//
//

#pragma once

#include "GridController.h"

class ComboGridController : public IDrawableModule, public IGridController, public IGridControllerListener
{
public:
   ComboGridController();
   ~ComboGridController() {}
   static IDrawableModule* Create() { return new ComboGridController(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;

   void SetGridControllerOwner(IGridControllerListener* owner) override { mOwner = owner; }
   void SetLight(int x, int y, GridColor color, bool force = false) override;
   void SetLightDirect(int x, int y, int color, bool force = false) override;
   void ResetLights() override;
   int NumCols() override { return mCols; }
   int NumRows() override { return mRows; }
   bool HasInput() const override;
   bool IsConnected() const override { return true; }

   void OnControllerPageSelected() override {}
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   enum Arrangements
   {
      kHorizontal,
      kVertical,
      kSquare
   };

   void InitializeCombo();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   unsigned int mRows{ 0 };
   unsigned int mCols{ 0 };
   std::vector<IGridController*> mGrids;
   Arrangements mArrangement{ Arrangements::kHorizontal };
   IGridControllerListener* mOwner{ nullptr };
};
