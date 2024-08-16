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
//  GridController.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#include "GridController.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

GridControlTarget::GridControlTarget(IGridControllerListener* owner, const char* name, int x, int y)
: mOwner(owner)
{
   SetName(name);
   SetPosition(x, y);
   dynamic_cast<IDrawableModule*>(owner)->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

void GridControlTarget::Render()
{
   ofPushStyle();

   ofNoFill();
   ofSetLineWidth(2);
   ofSetColor(200, 200, 200, gModuleDrawAlpha);
   ofCircle(mX + 6, mY + 8, 5);

   DrawGridIcon(mX + 15, mY + 2);

   DrawPatchCableHover();

   ofPopStyle();
}

//static
void GridControlTarget::DrawGridIcon(float x, float y)
{
   ofPushStyle();

   ofSetLineWidth(1);
   float gridSize = 12;
   ofRect(x, y, gridSize, gridSize, 0);
   ofRect(x + gridSize / 3, y, gridSize / 3, 12, 0);
   ofRect(x, y + gridSize / 3, 12, gridSize / 3, 0);

   ofPopStyle();
}

bool GridControlTarget::CanBeTargetedBy(PatchCableSource* source) const
{
   return source->GetConnectionType() == kConnectionType_Grid;
}

bool GridControlTarget::MouseMoved(float x, float y)
{
   CheckHover(x, y);
   return false;
}

namespace
{
   const int kSaveStateRev = 1;
}

void GridControlTarget::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
}

void GridControlTarget::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}

//----------------

GridControllerMidi::GridControllerMidi()
{
}

void GridControllerMidi::OnControllerPageSelected()
{
   mOwner->OnControllerPageSelected();

   for (int i = 0; i < mCols; ++i)
   {
      for (int j = 0; j < mRows; ++j)
      {
         SetLightDirect(i, j, mLights[i][j], K(force));
      }
   }
}

void GridControllerMidi::OnInput(int control, float velocity)
{
   int x = 0;
   int y = 0;
   bool found = false;
   for (x = 0; x < mCols; ++x)
   {
      for (y = 0; y < mRows; ++y)
      {
         if (mControls[x][y] == control)
         {
            found = true;
            break;
         }
      }
      if (found)
         break;
   }

   if (found)
   {
      mInput[x][y] = velocity;

      if (mOwner)
         mOwner->OnGridButton(x, y, velocity, this);
   }
}

bool GridControllerMidi::HasInput() const
{
   for (int i = 0; i < mCols; ++i)
   {
      for (int j = 0; j < mRows; ++j)
      {
         if (mInput[i][j] > 0)
         {
            return true;
         }
      }
   }

   return false;
}

void GridControllerMidi::SetLight(int x, int y, GridColor color, bool force)
{
   if (x >= mCols || y >= mRows)
      return;

   int colorIdx = (int)color;
   int rawColor = 0;
   if (mColors.size())
   {
      if (colorIdx >= mColors.size()) //we don't have this many colors
      {
         while (colorIdx >= mColors.size())
            colorIdx -= 2; //move back by two to retain bright/dimness
         if (colorIdx <= 0)
            colorIdx = 1; //never set a non-off light to "off"
      }
      rawColor = mColors[colorIdx];
   }
   else
   {
      rawColor = colorIdx > 0 ? 127 : 0;
   }

   SetLightDirect(x, y, rawColor, force);
}

void GridControllerMidi::SetLightDirect(int x, int y, int color, bool force)
{
   if (mLights[x][y] != color || force)
   {
      if (mMidiController)
      {
         if (mMessageType == kMidiMessage_Note)
            mMidiController->SendNote(mControllerPage, mControls[x][y], color);
         else if (mMessageType == kMidiMessage_Control)
            mMidiController->SendCC(mControllerPage, mControls[x][y], color);
      }
      mLights[x][y] = color;
   }
}

void GridControllerMidi::ResetLights()
{
   for (int i = 0; i < mCols; ++i)
   {
      for (int j = 0; j < mRows; ++j)
      {
         SetLight(i, j, kGridColorOff);
      }
   }
}

void GridControllerMidi::SetUp(GridLayout* layout, int page, MidiController* controller)
{
   mRows = layout->mRows;
   mCols = layout->mCols;
   for (unsigned int row = 0; row < mRows; ++row)
   {
      for (unsigned int col = 0; col < mCols; ++col)
      {
         int index = col + row * mCols;
         mControls[col][row] = layout->mControls[index];
      }
   }

   mColors = layout->mColors;

   mMessageType = layout->mType;

   mMidiController = controller;
   mControllerPage = page;

   OnControllerPageSelected();
}

void GridControllerMidi::UnhookController()
{
   mMidiController = nullptr;
}
