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

    GridModule.h
    Created: 19 Jul 2020 10:36:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "MidiDevice.h"
#include "GridController.h"
#include "UIGrid.h"

class ScriptModule;

class GridModule : public IDrawableModule, public IGridControllerListener, public UIGridListener, public INoteReceiver, public IGridController
{
public:
   GridModule();
   ~GridModule();
   static IDrawableModule* Create() { return new GridModule(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }
   void CreateUIControls() override;

   void Init() override;

   void SetGrid(int cols, int rows) { mGrid->SetGrid(cols, rows); }
   void SetLabel(int row, std::string label);
   void Set(int col, int row, float value)
   {
      mGrid->SetVal(col, row, value, !K(notifyListener));
      UpdateLights();
   }
   float Get(int col, int row) { return mGrid->GetVal(col, row); }
   void HighlightCell(int col, int row, double time, double duration, int colorIndex);
   void SetDivision(int steps) { return mGrid->SetMajorColSize(steps); }
   int GetCols() const { return mGrid->GetCols(); }
   int GetRows() const { return mGrid->GetRows(); }
   void SetColor(int colorIndex, ofColor color);
   void SetMomentary(bool momentary) { mGrid->SetMomentary(momentary); }
   void SetCellColor(int col, int row, int colorIndex);
   int GetCellColor(int col, int row) { return mGridOverlay[row * kGridOverlayMaxDim + col]; }
   void AddListener(ScriptModule* listener);
   void Clear();

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   //IGridController
   void SetGridControllerOwner(IGridControllerListener* owner) override { mGridControllerOwner = owner; }
   void SetLight(int x, int y, GridColor color, bool force = false) override;
   void SetLightDirect(int x, int y, int color, bool force = false) override;
   void ResetLights() override;
   int NumCols() override { return GetCols(); }
   int NumRows() override { return GetRows(); }
   bool HasInput() const override;
   bool IsConnected() const override { return true; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 4; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   void MouseReleased() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   ofColor GetColor(int colorIndex) const;
   void UpdateLights();

   GridControlTarget* mGridControlTarget{ nullptr };
   PatchCableSource* mGridOutputCable{ nullptr };
   IGridControllerListener* mGridControllerOwner{ nullptr };

   UIGrid* mGrid{ nullptr };
   std::vector<std::string> mLabels;
   std::vector<ofColor> mColors;

   struct HighlightCellElement
   {
      double time{ -1 };
      Vec2i position;
      double duration{ 0 };
      ofColor color;
   };
   std::array<HighlightCellElement, 50> mHighlightCells;

   static const int kGridOverlayMaxDim = 256;
   std::array<int, kGridOverlayMaxDim * kGridOverlayMaxDim> mGridOverlay;

   std::list<ScriptModule*> mScriptListeners;

   Checkbox* mMomentaryCheckbox{ nullptr };
   bool mMomentary{ false };
   bool mDirectColorMode{ true };
};
