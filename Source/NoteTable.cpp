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
//  NoteTable.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/1/21
//
//

#include "NoteTable.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "NoteTable.h"
#include "LaunchpadInterpreter.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"
#include "MathUtils.h"

NoteTable::NoteTable()
: mLength(8)
, mLengthSlider(nullptr)
, mGrid(nullptr)
, mOctave(3)
, mOctaveSlider(nullptr)
, mNoteMode(kNoteMode_Scale)
, mNoteModeSelector(nullptr)
, mNoteRange(15)
, mShowColumnControls(false)
, mRowOffset(0)
, mSetLength(false)
, mRandomizePitchButton(nullptr)
, mRandomizePitchChance(1)
, mRandomizePitchRange(1)
, mGridControlOffsetX(0)
, mGridControlOffsetY(0)
{  
   RandomizePitches(true);
   
   TheScale->AddListener(this);
}

void NoteTable::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   float width, height;
   UIBLOCK(140);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, kMaxLength); UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mOctaveSlider, "octave", &mOctave, 0, 7); UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mNoteModeSelector, "notemode", (int*)(&mNoteMode), 80);
   UIBLOCK_NEWLINE();
   BUTTON(mRandomizePitchButton, "random pitch"); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mRandomizePitchChanceSlider, "rand pitch chance", &mRandomizePitchChance, 0, 1, 2); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mRandomizePitchRangeSlider, "rand pitch range", &mRandomizePitchRange, 0, 1, 2);
   UIBLOCK_NEWLINE();
   UICONTROL_CUSTOM(mGridControlTarget, new GridControlTarget(UICONTROL_BASICS("grid"))); UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mGridControlOffsetXSlider, "x offset", &mGridControlOffsetX, 0, 16); UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mGridControlOffsetYSlider, "y offset", &mGridControlOffsetY, 0, 16);
   ENDUIBLOCK(width, height);

   mGrid = new UIGrid(5, height + 18, width-10, 110, 8, 24, this);
   
   for (int i=0; i< kMaxLength; ++i)
   {
      mToneDropdowns[i] = new DropdownList(this,("tone"+ofToString(i)).c_str(),-1,-1,&(mTones[i]),40);
      mToneDropdowns[i]->SetDrawTriangle(false);
      mToneDropdowns[i]->SetShowing(false);
   }
   SetUpColumnControls();
   
   mNoteModeSelector->AddLabel("scale", kNoteMode_Scale);
   mNoteModeSelector->AddLabel("chromatic", kNoteMode_Chromatic);
   mNoteModeSelector->AddLabel("pentatonic", kNoteMode_Pentatonic);
   mNoteModeSelector->AddLabel("5ths", kNoteMode_Fifths);
   
   mGrid->SetSingleColumnMode(true);
   mGrid->SetFlip(true);
   mGrid->SetListener(this);

   for (int i = 0; i < kMaxLength; ++i)
   {
      mColumnCables[i] = new AdditionalNoteCable();
      mColumnCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mColumnCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(0, 1));
      AddPatchCableSource(mColumnCables[i]->GetPatchCableSource());
   }
}

NoteTable::~NoteTable()
{
   TheScale->RemoveListener(this);
}

void NoteTable::Init()
{
   IDrawableModule::Init();
   
   SyncGridToSeq();
}

void NoteTable::Poll()
{
   UpdateGridControllerLights(false);
}

void NoteTable::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mGridControlOffsetXSlider->SetShowing(mGridControlTarget->GetGridController() != nullptr && mLength > mGridControlTarget->GetGridController()->NumCols());
   mGridControlOffsetYSlider->SetShowing(mGridControlTarget->GetGridController() != nullptr && mNoteRange > mGridControlTarget->GetGridController()->NumRows());
   
   mLengthSlider->Draw();
   mOctaveSlider->Draw();
   mNoteModeSelector->Draw();
   mRandomizePitchButton->Draw();
   mGridControlTarget->Draw();
   mGridControlOffsetXSlider->Draw();
   mGridControlOffsetYSlider->Draw();
   mRandomizePitchChanceSlider->Draw();
   mRandomizePitchRangeSlider->Draw();
   
   mGrid->Draw();
   
   ofPushStyle();
   ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
   for (int i = 0; i < mGrid->GetCols(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(i, mGrid->GetRows()-1) + mGrid->GetPosition(true);
      DrawTextNormal(ofToString(i), pos.x + 1, pos.y - 7);
   }
   for (int i=0; i<mGrid->GetRows(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(0, i-1) + mGrid->GetPosition(true);
      float scale = MIN(mGrid->IClickable::GetDimensions().y / mGrid->GetRows(), 20);
      DrawTextNormal(NoteName(RowToPitch(i),false,true) + "("+ ofToString(RowToPitch(i)) + ")", pos.x + 4, pos.y - (scale/8), scale);
   }
   ofPopStyle();
   
   if (mGridControlTarget->GetGridController())
   {
      int controllerCols = mGridControlTarget->GetGridController()->NumCols();
      int controllerRows = mGridControlTarget->GetGridController()->NumRows();
      
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(4);
      ofSetColor(255,0,0,50);
      float squareh = float(mGrid->GetHeight())/mNoteRange;
      float squarew = float(mGrid->GetWidth())/mLength;
      ofRectangle gridRect = mGrid->GetRect(K(local));
      ofRect(gridRect.x + squarew * mGridControlOffsetX,
             gridRect.y + gridRect.height - squareh * (mGridControlOffsetY + controllerRows),
             squarew * controllerCols,
             squareh * controllerRows);
      ofPopStyle();
   }
   
   ofPushStyle();
   ofFill();
   float gridX, gridY, gridW, gridH;
   mGrid->GetPosition(gridX, gridY, true);
   gridW = mGrid->GetWidth();
   gridH = mGrid->GetHeight();
   float boxHeight = (float(gridH)/mNoteRange);
   float boxWidth = (float(gridW)/mGrid->GetCols());
   
   for (int i=0;i<mNoteRange;++i)
   {
      if (RowToPitch(i)%TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot()%TheScale->GetPitchesPerOctave())
         ofSetColor(0,255,0,80);
      else if (TheScale->GetPitchesPerOctave() == 12 && RowToPitch(i)%TheScale->GetPitchesPerOctave() == (TheScale->ScaleRoot()+7)%TheScale->GetPitchesPerOctave())
         ofSetColor(200,150,0,80);
      else if (mNoteMode == kNoteMode_Chromatic && TheScale->IsInScale(RowToPitch(i)))
         ofSetColor(100,75,0,80);
      else
         continue;
      
      float y = gridY + gridH - i*boxHeight;
      ofRect(gridX,y-boxHeight,gridW,boxHeight);
   }
   
   for (int i=0; i<mGrid->GetCols(); ++i)
   {
      const float kPlayHighlightDurationMs = 250;
      if (mLastColumnPlayTime[i] != -1)
      {
         if (gTime - mLastColumnPlayTime[i] < kPlayHighlightDurationMs)
         {
            if (gTime - mLastColumnPlayTime[i] > 0)
            {
               float fade = (1 - (gTime - mLastColumnPlayTime[i]) / kPlayHighlightDurationMs);
               ofPushStyle();
               ofNoFill();
               ofSetLineWidth(3 * fade);
               ofVec2f pos = mGrid->GetCellPosition(i, mTones[i]) + mGrid->GetPosition(true);
               float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
               float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();
               ofSetColor(ofColor::white, fade * 255);
               ofRect(pos.x, pos.y, xsize, ysize);
               ofPopStyle();
            }
         }
         else
         {
            mLastColumnPlayTime[i] = -1;
         }
      }
   }
   
   float controlYPos = gridY+gridH;
   float moduleWidth, moduleHeight;
   GetModuleDimensions(moduleWidth, moduleHeight);
   for (int i=0; i< kMaxLength; ++i)
   {
      if (i < mLength)
      {
         mToneDropdowns[i]->SetShowing(mShowColumnControls);
         mToneDropdowns[i]->SetPosition(gridX+boxWidth*i, controlYPos);
         mToneDropdowns[i]->SetWidth(boxWidth);
         mToneDropdowns[i]->Draw();
      }
      else
      {
         mToneDropdowns[i]->SetShowing(false);
      }

      if (i < mLength && mShowColumnControls)
      {
         ofVec2f pos = mGrid->GetCellPosition(i, 0) + mGrid->GetPosition(true);
         pos.x += mGrid->GetWidth() / float(mLength) * .5f;
         pos.y = moduleHeight - 7;
         mColumnCables[i]->GetPatchCableSource()->SetManualPosition(pos.x, pos.y);
         mColumnCables[i]->GetPatchCableSource()->SetEnabled(true);
      }
      else
      {
         mColumnCables[i]->GetPatchCableSource()->SetEnabled(false);
      }
   }
   
   ofPopStyle();
}

void NoteTable::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mGrid->TestClick(x,y,right);
}

void NoteTable::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool NoteTable::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   return false;
}

void NoteTable::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteTable::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
      for (int i=0; i<mGrid->GetCols(); ++i)
      {
         bool colHasPitch = false;
         for (int j=0; j<mGrid->GetRows(); ++j)
         {
            float val = mGrid->GetVal(i,j);
            if (val > 0)
            {
               mTones[i] = j;
               colHasPitch = true;
               break;
            }
         }

         if (!colHasPitch)
            mTones[i] = -1;
      }
   }
}

int NoteTable::RowToPitch(int row)
{
   row += mRowOffset;
   
   int numPitchesInScale = TheScale->NumTonesInScale();
   switch (mNoteMode)
   {
      case kNoteMode_Scale:
         return TheScale->GetPitchFromTone(row+mOctave*numPitchesInScale+TheScale->GetScaleDegree());
      case kNoteMode_Chromatic:
         return row + mOctave * TheScale->GetPitchesPerOctave();
      case kNoteMode_Pentatonic:
      {
         bool isMinor = TheScale->IsInScale(TheScale->ScaleRoot() + 3);
         const int minorPentatonic[5] = { 0, 3, 5, 7, 10 };
         const int majorPentatonic[5] = { 0, 2, 4, 7, 9 };

         if (isMinor)
            return TheScale->ScaleRoot() + (row / 5 + mOctave) * TheScale->GetPitchesPerOctave() + minorPentatonic[row % 5];
         else
            return TheScale->ScaleRoot() + (row / 5 + mOctave) * TheScale->GetPitchesPerOctave() + majorPentatonic[row % 5];

      }
      case kNoteMode_Fifths:
      {
         int oct = (row/2)*numPitchesInScale;
         bool isFifth = row%2 == 1;
         int fifths = oct;
         if (isFifth)
            fifths += 4;
         return TheScale->GetPitchFromTone(fifths+mOctave*numPitchesInScale+TheScale->GetScaleDegree());
      }
   }
   return row;
}

int NoteTable::PitchToRow(int pitch)
{
   for (int i=0; i<mGrid->GetRows(); ++i)
   {
      if (pitch == RowToPitch(i))
         return i;
   }
   return -1;
}

float NoteTable::ExtraWidth() const
{
   return 10;
}

float NoteTable::ExtraHeight() const
{
   float height = 77;
   if (mShowColumnControls)
      height += 22;
   return height;
}

void NoteTable::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + ExtraWidth();
   height = mGrid->GetHeight() + ExtraHeight();
}

void NoteTable::Resize(float w, float h)
{
   mGrid->SetDimensions(MAX(w - ExtraWidth(), 210), MAX(h - ExtraHeight(), 80));
}

void NoteTable::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled && pitch < kMaxLength &&
       ((pitch < mLength || mTones[pitch] > -1) || velocity == 0))  //still allow note-offs through
      PlayColumn(time, pitch, velocity, voiceIdx, modulation);
}

void NoteTable::PlayColumn(double time, int column, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int outputPitch = RowToPitch(mTones[column]);
   if (velocity == 0)
      outputPitch = mLastColumnNoteOnPitch[column];
   PlayNoteOutput(time, outputPitch, velocity, voiceIdx, modulation);
   if (velocity > 0)
   {
      mLastColumnPlayTime[column] = time;
      mLastColumnNoteOnPitch[column] = outputPitch;
   }
   mColumnCables[column]->PlayNoteOutput(time, outputPitch, velocity, voiceIdx, modulation);
}

void NoteTable::UpdateGridControllerLights(bool force)
{
   if (mGridControlTarget->GetGridController())
   {
      for (int x=0; x<mGridControlTarget->GetGridController()->NumCols(); ++x)
      {
         for (int y=0; y<mGridControlTarget->GetGridController()->NumRows(); ++y)
         {
            int column = x + mGridControlOffsetX;
            int row = y - mGridControlOffsetY;
            
            GridColor color = GridColor::kGridColorOff;
            if (column < mLength)
            {
               if (mTones[column] == mGridControlTarget->GetGridController()->NumRows() - 1 - row)
               {
                  if (mLastColumnPlayTime[column] + 80 > gTime)
                     color = GridColor::kGridColor3Bright;
                  else
                     color = GridColor::kGridColor1Bright;
               }
            }
            mGridControlTarget->GetGridController()->SetLight(x, y, color, force);
         }
      }
   }
}

void NoteTable::OnControllerPageSelected()
{
   UpdateGridControllerLights(true);
}

void NoteTable::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   int col = x + mGridControlOffsetX;
   int row = y - mGridControlOffsetY;
   if (grid == mGridControlTarget->GetGridController() && col >= 0 && col < mLength && velocity > 0)
   {
      int tone = mGridControlTarget->GetGridController()->NumRows() - 1 - row;
      mTones[col] = tone;
      SyncGridToSeq();
      UpdateGridControllerLights(false);
   }
}

void NoteTable::ButtonClicked(ClickButton* button)
{
   if (button == mRandomizePitchButton)
   {
      RandomizePitches(GetKeyModifiers() & kModifier_Shift);
      SyncGridToSeq();
   }
}

void NoteTable::RandomizePitches(bool fifths)
{
   if (fifths)
   {
      for (int i=0; i < mLength; ++i)
      {
         if (ofRandom(1) <= mRandomizePitchChance)
         {
            switch (gRandom() % 5)
            {
               case 0:
                  mTones[i] = 0;
                  break;
               case 1:
                  mTones[i] = 4;
                  break;
               case 2:
                  mTones[i] = 7;
                  break;
               case 3:
                  mTones[i] = 11;
                  break;
               case 4:
                  mTones[i] = 14;
                  break;
            }
         }
      }
   }
   else
   {
      for (int i=0; i < mLength; ++i)
      {
         if (ofRandom(1) <= mRandomizePitchChance)
         {
            float minValue = MAX(0, mTones[i] - mNoteRange * mRandomizePitchRange);
            float maxValue = MIN(mNoteRange, mTones[i] + mNoteRange * mRandomizePitchRange);
            if (minValue != maxValue)
               mTones[i] = ofClamp(int(ofRandom(minValue, maxValue) + .5f), 0, mNoteRange-1);
         }
      }
   }
}

void NoteTable::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mNoteModeSelector)
   {
      if (mNoteMode != oldVal)
         mRowOffset = 0;
      SetUpColumnControls();
   }
   for (int i=0; i<kMaxLength; ++i)
   {
      if (list == mToneDropdowns[i])
         SyncGridToSeq();
   }
}

void NoteTable::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLengthSlider)
   {
      mLength = MIN(mLength, kMaxLength);

      if (mLength > oldVal)
      {
         //slice the loop into the nearest power of 2 and loop new steps from there
         int oldLengthPow2 = std::max(1, MathUtils::HighestPow2(oldVal));
         for (int i = oldVal; i < mLength; ++i)
         {
            int loopedFrom = i % oldLengthPow2;
            mTones[i] = mTones[loopedFrom];
         }
      }

      mGrid->SetGrid(mLength, mNoteRange);
      SyncGridToSeq();
   }
   if (slider == mOctaveSlider)
      SetUpColumnControls();
   if (slider == mGridControlOffsetXSlider || slider == mGridControlOffsetYSlider)
      UpdateGridControllerLights(false);
}

void NoteTable::SyncGridToSeq()
{
   mGrid->Clear();
   for (int i=0; i<kMaxLength; ++i)
   {
      if (mTones[i] < 0)
         continue;

      mGrid->SetVal(i,mTones[i], 1,false);
   }
   mGrid->SetGrid(mLength, mNoteRange);
   if (mGridControlTarget->GetGridController())
   {
      int maxXOffset = mLength - mGridControlTarget->GetGridController()->NumCols();
      if (maxXOffset >= 0)
         mGridControlOffsetXSlider->SetExtents(0, maxXOffset);
      int maxYOffset = mNoteRange - mGridControlTarget->GetGridController()->NumRows();
      if (maxYOffset >= 0)
         mGridControlOffsetYSlider->SetExtents(0, maxYOffset);
      
      mGridControlOffsetX = MAX(MIN(mGridControlOffsetX, maxXOffset), 0);
      mGridControlOffsetY = MAX(MIN(mGridControlOffsetY, maxYOffset), 0);
   }
}

void NoteTable::OnScaleChanged()
{
   SetUpColumnControls();
}

void NoteTable::SetUpColumnControls()
{
   if (TheSynth->IsLoadingModule())
      return;
   
   for (int i=0; i<kMaxLength; ++i)
   {
      mToneDropdowns[i]->Clear();
      for (int j=0; j<mNoteRange; ++j)
         mToneDropdowns[i]->AddLabel(NoteName(RowToPitch(j), false, true), j);
   }
}

void NoteTable::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void NoteTable::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   mModuleSaveData.LoadInt("gridrows", moduleInfo, 15, 1, 127, K(isTextField));
   mModuleSaveData.LoadBool("columncontrols", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteTable::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mNoteRange = mModuleSaveData.GetInt("gridrows");
   mShowColumnControls = mModuleSaveData.GetBool("columncontrols");
   SyncGridToSeq();
   SetUpColumnControls();
}

namespace
{
   const int kSaveStateRev = 2;
}

void NoteTable::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mGrid->SaveState(out);
}

void NoteTable::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   mGrid->LoadState(in);
   GridUpdated(mGrid, 0, 0, 0, 0);
}
