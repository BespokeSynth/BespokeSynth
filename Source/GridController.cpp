//
//  GridController.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#include "GridController.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

GridController::GridController(IGridControllerListener* owner, const char* name, int x, int y)
: mMessageType(kMidiMessage_Note)
, mController(nullptr)
, mControllerPage(0)
, mRows(8)
, mCols(8)
, mOwner(owner)
{
   SetName(name);
   SetPosition(x,y);
   dynamic_cast<IDrawableModule*>(owner)->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
   
   bzero(mControls, sizeof(int)*MAX_GRIDCONTROLLER_ROWS*MAX_GRIDCONTROLLER_COLS);
   bzero(mInput, sizeof(float)*MAX_GRIDCONTROLLER_ROWS*MAX_GRIDCONTROLLER_COLS);
   bzero(mLights, sizeof(int)*MAX_GRIDCONTROLLER_ROWS*MAX_GRIDCONTROLLER_COLS);
}

void GridController::Render()
{
   ofPushStyle();
   
   ofNoFill();
   ofSetLineWidth(3);
   ofSetColor(200,200,200,gModuleDrawAlpha);
   ofCircle(mX+6,mY+8,5);
   
   ofSetLineWidth(1);
   float gridOffsetX = 15;
   float gridOffsetY = 2;
   float gridSize = 12;
   ofRect(mX+gridOffsetX, mY+gridOffsetY, gridSize, gridSize, 0);
   ofRect(mX+gridOffsetX+gridSize/3, mY+gridOffsetY, gridSize/3, 12, 0);
   ofRect(mX+gridOffsetX, mY+gridOffsetY+gridSize/3, 12, gridSize/3, 0);
   
   DrawPatchCableHover();
   
   ofPopStyle();
}

void GridController::OnControllerPageSelected()
{
   mOwner->OnControllerPageSelected();
   
   for (int i=0; i<mCols; ++i)
   {
      for (int j=0; j<mRows; ++j)
      {
         SetLightDirect(i, j, mLights[i][j], K(force));
      }
   }
}

void GridController::OnInput(int control, float velocity)
{
   int x;
   int y;
   bool found = false;
   for (x=0;x<mCols;++x)
   {
      for (y=0;y<mRows;++y)
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

bool GridController::HasInput() const
{
   for (int i=0; i<mCols; ++i)
   {
      for (int j=0; j<mRows; ++j)
      {
         if (mInput[i][j] > 0)
         {
            return true;
         }
      }
   }
   
   return false;
}

void GridController::SetLight(int x, int y, GridColor color, bool force)
{
   if (x >= mCols || y >= mRows)
      return;
   
   int colorIdx = (int)color;
   int rawColor = 0;
   if (mColors.size())
   {
      if (colorIdx >= mColors.size())  //we don't have this many colors
      {
         while (colorIdx >= mColors.size())
            colorIdx -= 2; //move back by two to retain bright/dimness
         if (colorIdx <= 0)
            colorIdx = 1;  //never set a non-off light to "off"
      }
      rawColor = mColors[colorIdx];
   }
   
   SetLightDirect(x, y, rawColor, force);
}

void GridController::SetLightDirect(int x, int y, int color, bool force)
{
   if (mLights[x][y] != color || force)
   {
      if (mController)
      {
         if (mMessageType == kMidiMessage_Note)
            mController->SendNote(mControllerPage, mControls[x][y], color);
         else if (mMessageType == kMidiMessage_Control)
            mController->SendCC(mControllerPage, mControls[x][y], color);
      }
      mLights[x][y] = color;
   }
}

void GridController::ResetLights()
{
   for (int i=0; i<mCols; ++i)
   {
      for (int j=0; j<mRows; ++j)
      {
         SetLight(i,j,kGridColorOff);
      }
   }
}

void GridController::SetUp(GridLayout* layout, int page, MidiController* controller)
{
   mRows = layout->mRows;
   mCols = layout->mCols;
   for (unsigned int row=0; row<mRows; ++row)
   {
      for (unsigned int col=0; col<mCols; ++col)
      {  
         int index = col + row * mCols;
         mControls[col][row] = layout->mControls[index];
      }
   }
   
   mColors = layout->mColors;
   
   mMessageType = layout->mType;
   
   mController = controller;
   mControllerPage = page;
   
   OnControllerPageSelected();
}

void GridController::UnhookController()
{
   mController = nullptr;
}

bool GridController::CanBeTargetedBy(PatchCableSource* source) const
{
   return source->GetConnectionType() == kConnectionType_Grid;
}

namespace
{
   const int kSaveStateRev = 1;
}

void GridController::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
}

void GridController::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}
