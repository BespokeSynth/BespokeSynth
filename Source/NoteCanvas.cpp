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
//  NoteCanvas.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#include "NoteCanvas.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "CanvasControls.h"
#include "Scale.h"
#include "CanvasElement.h"
#include "Profiler.h"
#include "CanvasTimeline.h"
#include "CanvasScrollbar.h"
#include "FillSaveDropdown.h"

#include "juce_gui_basics/juce_gui_basics.h"

NoteCanvas::NoteCanvas()
: IDrawableModule(410, 363)
{
   mVoiceModulations.resize(kNumVoices + 1);
}

void NoteCanvas::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void NoteCanvas::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mQuantizeButton = new ClickButton(this, "quantize", 160, 5);
   mSaveMidiButton = new ClickButton(this, "save midi", 228, 5);
   mLoadMidiButton = new ClickButton(this, "load midi", 290, 5);
   mLoadMidiTrackEntry = new TextEntry(this, "loadtrack", 350, 5, 3, &mLoadMidiTrack, 0, 127);
   //mClipButton = new ClickButton(this,"clip",220,5);
   mPlayCheckbox = new Checkbox(this, "play", 5, 5, &mPlay);
   mRecordCheckbox = new Checkbox(this, "rec", 50, 5, &mRecord);
   mFreeRecordCheckbox = new Checkbox(this, "free rec", 90, 5, &mFreeRecord);
   mNumMeasuresSlider = new IntSlider(this, "measures", 5, 25, 100, 15, &mNumMeasures, 1, 16);
   mShowIntervalsCheckbox = new Checkbox(this, "show chord intervals", 160, 25, &mShowIntervals);

   mIntervalSelector = new DropdownList(this, "interval", 110, 25, (int*)(&mInterval));
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);

   mCanvas = new Canvas(this, 5, 55, 390, 200, L(length, 1), L(rows, 128), L(cols, 16), &(NoteCanvasElement::Create));
   AddUIControl(mCanvas);
   mCanvas->SetNumVisibleRows(16);
   mCanvas->SetRowOffset(60);
   mCanvasControls = new CanvasControls();
   mCanvasControls->SetCanvas(mCanvas);
   mCanvasControls->CreateUIControls();
   AddChild(mCanvasControls);
   UpdateNumColumns();

   mCanvas->SetListener(this);

   mCanvasTimeline = new CanvasTimeline(mCanvas, "timeline");
   AddUIControl(mCanvasTimeline);

   mCanvasScrollbarHorizontal = new CanvasScrollbar(mCanvas, "scrollh", CanvasScrollbar::Style::kHorizontal);
   AddUIControl(mCanvasScrollbarHorizontal);

   mCanvasScrollbarVertical = new CanvasScrollbar(mCanvas, "scrollv", CanvasScrollbar::Style::kVertical);
   AddUIControl(mCanvasScrollbarVertical);
}

NoteCanvas::~NoteCanvas()
{
   mCanvas->SetListener(nullptr);
   TheTransport->RemoveAudioPoller(this);
}

void NoteCanvas::PlayNote(NoteMessage note)
{
   mNoteOutput.PlayNote(note);

   if (!mEnabled || !mRecord)
      return;

   if (mInputNotes[note.pitch]) //handle note-offs or retriggers
   {
      double endPos = GetCurPos(note.time);
      if (mInputNotes[note.pitch]->GetStart() > endPos)
         endPos += 1; //wrap
      mInputNotes[note.pitch]->SetEnd(endPos);
      mInputNotes[note.pitch] = nullptr;
   }

   if (note.velocity > 0)
   {
      if (mFreeRecord && mFreeRecordStartMeasure == -1)
         mFreeRecordStartMeasure = TheTransport->GetMeasure(note.time);

      double measurePos = GetCurPos(note.time) * mNumMeasures;
      NoteCanvasElement* element = AddNote(measurePos, note.pitch, note.velocity, 1 / mCanvas->GetNumCols(), note.voiceIdx, note.modulation);
      mInputNotes[note.pitch] = element;
      FitNotes(false, true);
   }
}

void NoteCanvas::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
}

bool NoteCanvas::FreeRecordParityMatched()
{
   int currentMeasureParity = (TheTransport->GetMeasure(gTime) % (mNumMeasures * 2)) / mNumMeasures;
   int recordStartMeasureParity = (mFreeRecordStartMeasure % (mNumMeasures * 2)) / mNumMeasures;
   return currentMeasureParity == recordStartMeasureParity;
}

void NoteCanvas::OnTransportAdvanced(float amount)
{
   PROFILER(NoteCanvas);

   if (mFreeRecord && mFreeRecordStartMeasure != -1)
   {
      if (TheTransport->GetMeasurePos(gTime) < amount &&
          !FreeRecordParityMatched())
      {
         int oldNumMeasures = mNumMeasures;
         while (!FreeRecordParityMatched())
            mNumMeasures *= 2;
         int shift = mFreeRecordStartMeasure % mNumMeasures - mFreeRecordStartMeasure % oldNumMeasures;
         SetNumMeasures(mNumMeasures);

         for (auto* element : mCanvas->GetElements())
            element->SetStart(element->GetStart() + float(shift) / mNumMeasures, true);
      }
   }

   if (mStopQueued)
   {
      mNoteOutput.Flush(NextBufferTime(false));
      for (int i = 0; i < mCurrentNotes.size(); ++i)
         mCurrentNotes[i] = nullptr;
      mStopQueued = false;
   }

   if (!mEnabled || !mPlay)
   {
      mCanvas->SetCursorPos(-1);
      return;
   }

   double cursorPlayTime = gTime;
   //if (Transport::sDoEventLookahead)??? should we?
   //   cursorPlayTime += Transport::sEventEarlyMs;
   //else
   cursorPlayTime += amount * TheTransport->MsPerBar();
   double curPos = GetCurPos(cursorPlayTime);

   mCanvas->FillElementsAt(curPos, mNoteChecker);
   for (int i = 0; i < 128; ++i)
   {
      int pitch = 127 - i;
      bool wasOn = mCurrentNotes[pitch] != nullptr || mInputNotes[pitch];
      bool nowOn = mNoteChecker[i] != nullptr || mInputNotes[pitch];
      bool hasChanged = (nowOn || wasOn) && mCurrentNotes[pitch] != static_cast<NoteCanvasElement*>(mNoteChecker[i]);

      if (wasOn && mInputNotes[pitch] == nullptr && hasChanged)
      {
         //note off
         if (mCurrentNotes[pitch])
         {
            double cursorAdvanceSinceEvent = curPos - mCurrentNotes[pitch]->GetEnd();
            if (cursorAdvanceSinceEvent < 0)
               cursorAdvanceSinceEvent += 1;
            double time = cursorPlayTime - cursorAdvanceSinceEvent * TheTransport->MsPerBar() * mNumMeasures;
            if (time < gTime)
               time = gTime;
            mNoteOutput.PlayNote(NoteMessage(time, pitch, 0, mCurrentNotes[pitch]->GetVoiceIdx()));
            mCurrentNotes[pitch] = nullptr;
         }
      }
      if (nowOn && mInputNotes[pitch] == nullptr && hasChanged && !mPitchMuted[pitch])
      {
         //note on
         NoteCanvasElement* note = static_cast<NoteCanvasElement*>(mNoteChecker[i]);
         assert(note);
         double cursorAdvanceSinceEvent = curPos - note->GetStart();
         if (cursorAdvanceSinceEvent < 0)
            cursorAdvanceSinceEvent += 1;
         double time = cursorPlayTime - cursorAdvanceSinceEvent * TheTransport->MsPerBar() * mNumMeasures;
         if (time > gTime)
         {
            mCurrentNotes[pitch] = note;
            mNoteOutput.PlayNote(NoteMessage(time, pitch, note->GetVelocity() * 127, note->GetVoiceIdx(), ModulationParameters(note->GetPitchBend(), note->GetModWheel(), note->GetPressure(), note->GetPan())));
         }
      }

      mNoteChecker[i] = nullptr;
   }

   for (int pitch = 0; pitch < 128; ++pitch)
   {
      if (mInputNotes[pitch])
      {
         float endPos = curPos;
         if (mInputNotes[pitch]->GetStart() > endPos)
            endPos += 1; //wrap
         mInputNotes[pitch]->SetEnd(endPos);

         int modIdx = mInputNotes[pitch]->GetVoiceIdx();
         if (modIdx == -1)
            modIdx = kNumVoices;
         float bend = ModulationParameters::kDefaultPitchBend;
         float mod = ModulationParameters::kDefaultModWheel;
         float pressure = ModulationParameters::kDefaultPressure;
         if (mVoiceModulations[modIdx].pitchBend)
            bend = mVoiceModulations[modIdx].pitchBend->GetValue(0);
         if (mVoiceModulations[modIdx].modWheel)
            mod = mVoiceModulations[modIdx].modWheel->GetValue(0);
         if (mVoiceModulations[modIdx].pressure)
            pressure = mVoiceModulations[modIdx].pressure->GetValue(0);
         mInputNotes[pitch]->WriteModulation(curPos, bend, mod, pressure, mVoiceModulations[modIdx].pan);
      }
      else if (mCurrentNotes[pitch])
      {
         mCurrentNotes[pitch]->UpdateModulation(curPos);
      }
   }
}

double NoteCanvas::GetCurPos(double time) const
{
   int loopMeasures = MAX(1, int(mCanvas->mLoopEnd - mCanvas->mLoopStart));
   return (((TheTransport->GetMeasure(time) % loopMeasures) + TheTransport->GetMeasurePos(time)) + mCanvas->mLoopStart) / mCanvas->GetLength();
}

void NoteCanvas::UpdateNumColumns()
{
   mCanvas->RescaleNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
   if (mInterval < kInterval_8n)
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval));
   else
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval) / 4);
}

void NoteCanvas::Clear(double time)
{
   bool wasPlaying = mPlay;
   mPlay = false;
   for (int pitch = 0; pitch < 128; ++pitch)
   {
      mInputNotes[pitch] = nullptr;
      mCurrentNotes[pitch] = nullptr;
   }
   mNoteOutput.Flush(time);
   mCanvas->Clear();
   mPlay = wasPlaying;
}

NoteCanvasElement* NoteCanvas::AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx /*=-1*/, ModulationParameters modulation /* = ModulationParameters()*/)
{
   double canvasPos = measurePos / mNumMeasures * mCanvas->GetNumCols();
   int col = int(canvasPos + .5f); //round off
   int row = mCanvas->GetNumRows() - pitch - 1;
   NoteCanvasElement* element = static_cast<NoteCanvasElement*>(mCanvas->CreateElement(col, row));
   element->mOffset = canvasPos - element->mCol; //the rounded off part
   element->mLength = length / mNumMeasures * mCanvas->GetNumCols();
   element->SetVelocity(velocity / 127.0f);
   element->SetVoiceIdx(voiceIdx);
   int modIdx = voiceIdx;
   if (modIdx == -1)
      modIdx = kNumVoices;
   mVoiceModulations[modIdx] = modulation;
   mCanvas->AddElement(element);

   return element;
}

void NoteCanvas::FitNotes(bool length /* = true*/, bool pitchRange /* = false*/)
{
   float latest = 0.0;
   int minPitch = 128;
   int maxPitch = -1;
   for (auto* element : mCanvas->GetElements())
   {
      NoteCanvasElement* noteElement = static_cast<NoteCanvasElement*>(element);
      if (noteElement->GetEnd() > latest)
         latest = element->GetEnd();
      if (noteElement->GetPitch() < minPitch)
         minPitch = noteElement->GetPitch();
      if (noteElement->GetPitch() > maxPitch)
         maxPitch = noteElement->GetPitch();
   }

   if (length)
      SetNumMeasures(static_cast<int>(std::ceil(latest)));

   if (pitchRange && maxPitch != -1)
   {
      int currentNumRows = mCanvas->GetNumVisibleRows();
      int newNumRows = std::max(currentNumRows, maxPitch - minPitch + 1);
      mCanvas->SetRowOffset(127 - minPitch - newNumRows + 1);
      mCanvas->SetNumVisibleRows(newNumRows);
   }
}

void NoteCanvas::CanvasUpdated(Canvas* canvas)
{
   if (canvas == mCanvas)
   {
   }
}

std::string NoteCanvas::GetCurrentEditMeasureString() const
{
   return "measure: " + ofToString(mEditMeasureOffset + 1) + " / " + ofToString(mNumMeasures);
}

bool NoteCanvas::OnAbletonGridControl(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue)
{
   /*if (controlIndex >= abletonGrid->GetGridStartIndex() && controlIndex < abletonGrid->GetGridStartIndex() + abletonGrid->GetGridNumPads())
   {
      int gridIndex = controlIndex - abletonGrid->GetGridStartIndex();
      int x = gridIndex % abletonGrid->GetGridNumCols();
      int y = abletonGrid->GetGridNumRows() - 1 - gridIndex / abletonGrid->GetGridNumCols();
      int index = x + y * abletonGrid->GetGridNumCols();

      // what should the grid do if there's no gridkeyboard?

      return true;
   }*/

   if (!mCurrentEditElements.empty())
   {
      if (controlIndex == AbletonDevice::kVolumeEncoderTurn ||
          controlIndex == AbletonDevice::kClickyEncoderTurn ||
          controlIndex == AbletonDevice::kOctaveUpButton ||
          controlIndex == AbletonDevice::kOctaveDownButton ||
          controlIndex == AbletonDevice::kPageLeftButton ||
          controlIndex == AbletonDevice::kPageRightButton)
      {
         for (auto* element : mCurrentEditElements)
         {
            if (controlIndex == AbletonDevice::kVolumeEncoderTurn)
            {
               float newVelocity = std::clamp(element->GetVelocity() + AbletonDevice::GetEncoderIncrement(midiValue), 0.01f, 1.0f);
               element->SetVelocity(newVelocity);
               mPlaceNoteVelocity = newVelocity;
            }

            if (controlIndex == AbletonDevice::kClickyEncoderTurn)
            {
               float length = element->mLength;
               int direction = midiValue <= 64 ? 1 : -1;
               if (abletonGrid->GetButtonState(AbletonDevice::kShiftButton) ||
                   abletonGrid->GetButtonState(AbletonDevice::kClickyEncoderButton))
               {
                  length = std::clamp(length + direction * 0.1f, 0.1f, mCanvas->GetNumCols() - (element->mCol + element->mOffset));
               }
               else
               {
                  float rounded = std::round(length);
                  if ((rounded < length && direction < 0) || (rounded > length && direction > 0))
                     length = rounded;
                  else
                     length = rounded + direction;
                  if (length <= 0)
                     length = 0.5f;
               }
               element->mLength = length;
            }

            if (midiValue > 0)
            {
               if (controlIndex == AbletonDevice::kOctaveUpButton ||
                   controlIndex == AbletonDevice::kOctaveDownButton)
               {
                  int direction = (controlIndex == AbletonDevice::kOctaveUpButton) ? 1 : -1;
                  bool octaveShift = abletonGrid->GetButtonState(AbletonDevice::kShiftButton);
                  int newPitch;
                  if (mGridKeyboardInterface != nullptr)
                     newPitch = mGridKeyboardInterface->TransposePitchInScale(element->GetPitch(), direction, octaveShift);
                  else
                     newPitch = element->GetPitch() + direction * (octaveShift ? 12 : 1);
                  element->SetPitch(std::clamp(newPitch, 0, 127));
               }

               if (controlIndex == AbletonDevice::kPageLeftButton ||
                   controlIndex == AbletonDevice::kPageRightButton)
               {
                  int direction = (controlIndex == AbletonDevice::kPageRightButton) ? 1 : -1;
                  float pos = element->mCol + element->mOffset;
                  if (abletonGrid->GetButtonState(AbletonDevice::kShiftButton))
                  {
                     pos += direction * 0.1f;
                  }
                  else
                  {
                     float rounded = std::round(pos);
                     if ((rounded < pos && direction < 0) || (rounded > pos && direction > 0))
                        pos = rounded;
                     else
                        pos = rounded + direction;
                  }
                  pos = std::clamp(pos, 0.0f, float(mCanvas->GetNumCols() - 1));
                  element->mCol = int(pos);
                  element->mOffset = pos - int(pos);
               }
            }
         }

         FitNotes(false, true);
         mHasMadeStepEdit = true;
         return true;
      }
   }
   else
   {
      if (controlIndex == AbletonDevice::kVolumeEncoderTurn && abletonGrid->GetButtonState(AbletonDevice::kLoopButton))
      {
         mEditMeasureOffsetSlider += AbletonDevice::GetEncoderIncrement(midiValue) * 10;
         if (mEditMeasureOffsetSlider > 1.0f)
         {
            mEditMeasureOffset = std::clamp(mEditMeasureOffset + 1, 0, mNumMeasures - 1);
            mEditMeasureOffsetSlider -= 1.0f;
         }
         else if (mEditMeasureOffsetSlider < -1.0f)
         {
            mEditMeasureOffset = std::clamp(mEditMeasureOffset - 1, 0, mNumMeasures - 1);
            mEditMeasureOffsetSlider += 1.0f;
         }
      }
   }

   int stepIndex = -1;
   if (abletonGrid->GetAbletonDeviceType() == AbletonDeviceType::Move)
      stepIndex = controlIndex - AbletonDevice::kStepButtonSection;
   if (stepIndex >= 0 && stepIndex < AbletonDevice::kNumStepButtons)
   {
      if (abletonGrid->GetButtonState(AbletonDevice::kLoopButton))
      {
         if (midiValue > 0) //button press
         {
            if (stepIndex < mNumMeasures)
               mEditMeasureOffset = stepIndex;
         }
      }
      else
      {
         if (midiValue > 0) //button press
         {
            mEditHoldStep = stepIndex;
            mEditHoldTime = gTime;
            mHasMadeStepEdit = false;

            int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop() / TheTransport->GetTimeSigBottom();
            int totalNumSteps = stepsPerMeasure * mNumMeasures;
            bool hasEditPitch = false;
            for (int measure = 0; measure < mNumMeasures; ++measure)
            {
               if (measure == mEditMeasureOffset || abletonGrid->GetButtonState(AbletonDevice::kShiftButton))
               {
                  float pos = (stepIndex + measure * stepsPerMeasure) / float(totalNumSteps);
                  float nextPos = (stepIndex + measure * stepsPerMeasure + 1) / float(totalNumSteps);
                  const auto& elements = mCanvas->GetElements();
                  for (auto* element : elements)
                  {
                     auto* noteElement = static_cast<NoteCanvasElement*>(element);
                     if (mEditCurrentPitchContext != -1 && noteElement->GetPitch() != mEditCurrentPitchContext)
                        continue;
                     if (noteElement->GetStart() >= pos && noteElement->GetStart() < nextPos)
                     {
                        mCurrentEditElements.push_back(noteElement);

                        if (noteElement->GetPitch() == mEditCurrentPitchContext && measure == mEditMeasureOffset)
                           hasEditPitch = true;
                     }
                  }
               }
            }

            if (mEditCurrentPitchContext != -1 && !hasEditPitch)
            {
               // turn note on
               AddEditPitch(mEditCurrentPitchContext, abletonGrid->GetButtonState(AbletonDevice::kShiftButton));
               mHasMadeStepEdit = true;
            }

            if (abletonGrid->GetButtonState(AbletonDevice::kCopyButton))
            {
               if (mClipboardElements.empty())
               {
                  //copy
                  CopyNotesToClipboard(stepIndex);
                  abletonGrid->DisplayScreenMessage(ofToString((int)mClipboardElements.size()) + " notes copied");
                  mHasMadeStepEdit = true;
               }
               else
               {
                  //paste
                  mCurrentEditElements.clear();
                  int pasteToStep = stepIndex + mEditMeasureOffset * stepsPerMeasure;
                  int pasteOffset = pasteToStep - mClipboardCopyFromStep;
                  for (auto* element : mClipboardElements)
                  {
                     auto* newNote = static_cast<NoteCanvasElement*>(element->CreateDuplicate());
                     newNote->mCol += pasteOffset;
                     mCurrentEditElements.push_back(newNote);
                     mCanvas->AddElement(newNote);
                  }
                  abletonGrid->DisplayScreenMessage(ofToString((int)mClipboardElements.size()) + " notes pasted");
                  mHasMadeStepEdit = true;
               }
            }

            if (abletonGrid->GetButtonState(AbletonDevice::kMoveDeleteButton))
            {
               for (auto* element : mCurrentEditElements)
                  mCanvas->RemoveElement(element);
               mCurrentEditElements.clear();
               mHasMadeStepEdit = true;
            }
         }
         else //button release
         {
            double holdTime = gTime - mEditHoldTime;
            if (holdTime < 500) //held for short time
            {
               if (!mHasMadeStepEdit && mEditCurrentPitchContext != -1)
               {
                  bool added = ToggleEditPitch(mEditCurrentPitchContext, abletonGrid->GetButtonState(AbletonDevice::kShiftButton));
                  if (added)
                     abletonGrid->DisplayScreenMessage("added " + NoteName(mEditCurrentPitchContext, false, true));
                  else
                     abletonGrid->DisplayScreenMessage("removed " + NoteName(mEditCurrentPitchContext, false, true));
               }
            }

            if (!abletonGrid->GetButtonState(AbletonDevice::kCopyButton))
               mCurrentEditElements.clear();

            mEditHoldStep = -1;
         }
      }

      return true;
   }

   if (controlIndex == AbletonDevice::kMoveDeleteButton)
   {
      if (midiValue > 0)
      {
         for (auto* element : mCurrentEditElements)
            mCanvas->RemoveElement(element);
         mCurrentEditElements.clear();
         mHasMadeStepEdit = true;
      }
      return true;
   }

   if (controlIndex == AbletonDevice::kPageRightButton)
   {
      if (midiValue > 0)
      {
         mEditMeasureOffset = std::clamp(mEditMeasureOffset + 1, 0, mNumMeasures - 1);
         abletonGrid->DisplayScreenMessage(GetCurrentEditMeasureString());
      }
      return true;
   }

   if (controlIndex == AbletonDevice::kPageLeftButton)
   {
      if (midiValue > 0)
      {
         mEditMeasureOffset = std::clamp(mEditMeasureOffset - 1, 0, mNumMeasures - 1);
         abletonGrid->DisplayScreenMessage(GetCurrentEditMeasureString());
      }
      return true;
   }

   if (abletonGrid->GetButtonState(AbletonDevice::kLoopButton) &&
       controlIndex == AbletonDevice::kClickyEncoderTurn)
   {
      int direction = midiValue <= 64 ? 1 : -1;
      SetNumMeasures(std::clamp(mNumMeasures + direction, mNumMeasuresSlider->GetMin(), mNumMeasuresSlider->GetMax()));
      return true;
   }

   if (controlIndex == AbletonDevice::kCopyButton)
   {
      if (midiValue > 0)
      {
         mCopyHoldTime = gTime;
         if (mEditHoldStep != -1 && !mCurrentEditElements.empty())
         {
            CopyNotesToClipboard(mEditHoldStep);
            abletonGrid->DisplayScreenMessage(ofToString((int)mClipboardElements.size()) + " notes copied");
         }
      }
      else
      {
         double holdTime = gTime - mCopyHoldTime;
         if (holdTime < 500) //held for short time
         {
            if (mClipboardElements.empty()) //never added anything to clipboard
            {
               DoubleLoop();
               abletonGrid->DisplayScreenMessage("loop doubled");
            }
         }
         mClipboardElements.clear();
         mCurrentEditElements.clear();
      }

      return true;
   }

   if (mGridKeyboardInterface != nullptr)
   {
      int pressedPitch = -1;
      if (controlIndex >= abletonGrid->GetGridStartIndex() && controlIndex < abletonGrid->GetGridStartIndex() + abletonGrid->GetGridNumPads())
      {
         int gridIndex = controlIndex - abletonGrid->GetGridStartIndex();
         int gridX = gridIndex % abletonGrid->GetGridNumCols();
         int gridY = abletonGrid->GetGridNumRows() - 1 - gridIndex / abletonGrid->GetGridNumCols();
         int pitch = mGridKeyboardInterface->GridToPitch(gridX, gridY);
         if (pitch >= 0 && pitch <= 127)
         {
            pressedPitch = pitch;
         }
      }

      if (pressedPitch != -1)
      {
         if (abletonGrid->GetButtonState(AbletonDevice::kShiftButton))
         {
            if (midiValue > 0)
            {
               if (mEditCurrentPitchContext == pressedPitch)
                  mEditCurrentPitchContext = -1;
               else
                  mEditCurrentPitchContext = pressedPitch;
            }

            return true;
         }

         if (abletonGrid->GetButtonState(AbletonDevice::kMoveMuteButton))
         {
            if (midiValue > 0)
               mPitchMuted[pressedPitch] = !mPitchMuted[pressedPitch];

            return true;
         }

         if (abletonGrid->GetButtonState(AbletonDevice::kMoveDeleteButton))
         {
            if (midiValue > 0)
            {
               std::list<CanvasElement*> toRemove;
               for (auto* element : mCanvas->GetElements())
               {
                  NoteCanvasElement* noteElement = static_cast<NoteCanvasElement*>(element);
                  if (noteElement->GetPitch() == pressedPitch)
                     toRemove.push_back(element);
               }

               for (auto* remove : toRemove)
                  mCanvas->RemoveElement(remove);

               mHasMadeStepEdit = true;
            }

            return true;
         }
      }

      if (mEditHoldStep != -1 && pressedPitch != -1)
      {
         if (midiValue > 0)
         {
            bool added = ToggleEditPitch(pressedPitch, abletonGrid->GetButtonState(AbletonDevice::kShiftButton));
            /*if (added)
               abletonGrid->DisplayScreenMessage("added " + NoteName(pressedPitch, false, true));
            else
               abletonGrid->DisplayScreenMessage("removed " + NoteName(pressedPitch, false, true));*/
         }
         return true;
      }

      return mGridKeyboardInterface->OnAbletonGridControl(abletonGrid, controlIndex, midiValue);
   }

   return false;
}

bool NoteCanvas::RemoveEditPitch(int pitch)
{
   bool removed = false;
   for (std::vector<NoteCanvasElement*>::iterator iter = mCurrentEditElements.begin(); iter != mCurrentEditElements.end();)
   {
      if ((*iter)->GetPitch() == pitch)
      {
         mCanvas->RemoveElement(*iter);
         iter = mCurrentEditElements.erase(iter);
         removed = true;
      }
      else
      {
         ++iter;
      }
   }
   return removed;
}

bool NoteCanvas::AddEditPitch(int pitch, bool atEveryMeasure)
{
   bool added = false;
   int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop() / TheTransport->GetTimeSigBottom();
   for (int measure = 0; measure < mNumMeasures; ++measure)
   {
      if (atEveryMeasure || mEditMeasureOffset == measure)
      {
         float pos = mEditHoldStep / float(stepsPerMeasure) + measure;
         float nextPos = (mEditHoldStep + 1) / float(stepsPerMeasure) + measure;

         //check to see if one already exists here, and don't add if it does
         bool alreadyHasStep = false;
         const auto& elements = mCanvas->GetElements();
         for (auto* element : elements)
         {
            auto* noteElement = static_cast<NoteCanvasElement*>(element);
            if (noteElement->GetPitch() == pitch &&
                noteElement->GetStart() * mNumMeasures >= pos &&
                noteElement->GetStart() * mNumMeasures < nextPos)
            {
               alreadyHasStep = true;
               break;
            }
         }

         if (!alreadyHasStep)
         {
            NoteCanvasElement* element = AddNote(pos, pitch, mPlaceNoteVelocity * 127, 1.0f / stepsPerMeasure, -1);
            mCurrentEditElements.push_back(element);
            added = true;
         }
      }
   }

   if (added)
      FitNotes(false, true);

   return added;
}

bool NoteCanvas::ToggleEditPitch(int pitch, bool atEveryMeasure)
{
   bool removed = RemoveEditPitch(pitch);

   if (removed)
      return false;
   else
      return AddEditPitch(pitch, atEveryMeasure);
}

void NoteCanvas::UpdateAbletonGridLeds(IAbletonGridDevice* abletonGrid)
{
   /*for (int x = 0; x < abletonGrid->GetGridNumCols(); ++x)
   {
      for (int y = 0; y < abletonGrid->GetGridNumRows(); ++y)
      {
         int pushColor = 0;
         int index = x + y * abletonGrid->GetGridNumCols();


         abletonGrid->SetLed(x + (7 - y) * 8 + abletonGrid->GetGridStartIndex(), pushColor);
      }
   }*/

   if (!mCurrentEditElements.empty())
   {
      abletonGrid->SetLed(AbletonDevice::kMoveDeleteButton, 127);
      abletonGrid->SetLed(AbletonDevice::kOctaveUpButton, 127, 1);
      abletonGrid->SetLed(AbletonDevice::kOctaveDownButton, 127, 1);
      abletonGrid->SetLed(AbletonDevice::kCopyButton, 127);
   }
   else
   {
      abletonGrid->SetLed(AbletonDevice::kMoveDeleteButton, 10);
      abletonGrid->SetLed(AbletonDevice::kOctaveUpButton, 10);
      abletonGrid->SetLed(AbletonDevice::kOctaveDownButton, 10);
      abletonGrid->SetLed(AbletonDevice::kCopyButton, 10);
   }

   abletonGrid->SetLed(AbletonDevice::kMoveMuteButton, 127);
   abletonGrid->SetLed(AbletonDevice::kLoopButton, 127);

   if (mGridKeyboardInterface != nullptr)
   {
      std::vector<int> heldPitches;
      for (const auto* element : mCurrentEditElements)
         heldPitches.push_back(element->GetPitch());
      mGridKeyboardInterface->SetPreviewNotes([heldPitches](const int pitch) -> bool
                                              {
                                                 return VectorContains(pitch, heldPitches);
                                              });

      mGridKeyboardInterface->UpdateAbletonGridLeds(abletonGrid);

      AbletonMoveControl* abletonMove = dynamic_cast<AbletonMoveControl*>(abletonGrid); //TODO(Ryan) make this not specific to move
      for (int x = 0; x < abletonMove->GetGridNumCols(); ++x)
      {
         for (int y = 0; y < abletonMove->GetGridNumRows(); ++y)
         {
            int pitch = mGridKeyboardInterface->GridToPitch(x, y);
            int ledIndex = x + (abletonMove->GetGridNumRows() - 1 - y) * abletonMove->GetGridNumCols() + abletonMove->GetGridStartIndex();

            if (pitch > 0 && pitch < (int)mPitchMuted.size() && mPitchMuted[pitch])
               abletonMove->SetLed(ledIndex, AbletonDevice::kColorDarkRed);

            int flashColor = -1;
            if (pitch == mEditCurrentPitchContext)
               flashColor = AbletonDevice::kColorLightBlue;
            abletonMove->SetLedFlashColor(ledIndex, flashColor);
         }
      }
   }

   if (abletonGrid->GetAbletonDeviceType() == AbletonDeviceType::Move)
   {
      float curPos = GetCurPos(gTime);
      const auto& elements = mCanvas->GetElements();
      int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop() / TheTransport->GetTimeSigBottom();
      int totalNumSteps = stepsPerMeasure * mNumMeasures;
      for (int step = 0; step < AbletonDevice::kNumStepButtons; ++step)
      {
         int pushColor = 0;

         if (abletonGrid->GetButtonState(AbletonDevice::kLoopButton) && mCurrentEditElements.empty())
         {
            if (step == mEditMeasureOffset)
               pushColor = AbletonDevice::kColorGreen;
            else if (step < mNumMeasures)
               pushColor = AbletonDevice::kColorDarkGreen;
         }
         else
         {
            float pos = (step + mEditMeasureOffset * stepsPerMeasure) / float(totalNumSteps);
            float nextPos = (step + mEditMeasureOffset * stepsPerMeasure + 1) / float(totalNumSteps);

            int desiredPitch = -1;
            //if (mGridKeyboardInterface != nullptr && mGridKeyboardInterface->IsDrumMode())
            desiredPitch = mEditCurrentPitchContext;
            bool hasNoteOn = false;
            bool hasNoteSustain = false;
            for (const auto* element : elements)
            {
               const auto* noteElement = static_cast<const NoteCanvasElement*>(element);
               if (desiredPitch == -1 || noteElement->GetPitch() == desiredPitch)
               {
                  if (noteElement->GetStart() >= pos && noteElement->GetStart() < nextPos)
                     hasNoteOn = true;
                  if (noteElement->GetStart() < nextPos && noteElement->GetEnd() > pos)
                     hasNoteSustain = true;
               }
            }

            if (hasNoteOn)
               pushColor = AbletonDevice::kColorWhite; //AbletonDevice::kColorPowderBlue;
            else if (hasNoteSustain)
               pushColor = AbletonDevice::kColorBlueEyes;
            else
               pushColor = AbletonDevice::kColorDarkGrey;

            if (curPos >= pos && curPos < nextPos)
               pushColor = AbletonDevice::kColorGreen;
         }

         abletonGrid->SetLed(step + AbletonDevice::kStepButtonSection, pushColor);
      }
   }
}

bool NoteCanvas::HasHighPriorityAbletonMoveScreenUpdate(IAbletonGridDevice* abletonGrid)
{
   if (!mCurrentEditElements.empty() && abletonGrid->GetButtonState(AbletonDevice::kClickyEncoderTouch))
      return true;
   if (abletonGrid->GetButtonState(AbletonDevice::kLoopButton))
      return true;
   return false;
}

bool NoteCanvas::UpdateAbletonMoveScreen(IAbletonGridDevice* abletonGrid, AbletonMoveLCD* lcd)
{
   if (!mCurrentEditElements.empty())
   {
      /*if (abletonGrid->GetButtonState(AbletonDevice::kVolumeEncoderTouch))
      {
         int y = 13;
         lcd->DrawText("velocity: ", 5, y, LCDFONT_STYLE_REGULAR);
         for (const auto* element : mCurrentEditElements)
         {
            lcd->DrawText(ofToString(element->GetVelocity(), 2).c_str(), 50, y, LCDFONT_STYLE_REGULAR);
            y += AbletonMoveLCD::kTextLineSpacing;
         }
      }
      else
      {
         lcd->DrawText(("holding " + ofToString((int)mCurrentEditElements.size()) + " elements").c_str(), 5, 13, LCDFONT_STYLE_REGULAR);
      }*/

      int y = 20;

      if (abletonGrid->GetButtonState(AbletonDevice::kOctaveUpButton) ||
          abletonGrid->GetButtonState(AbletonDevice::kOctaveDownButton))
         lcd->DrawRect(17, 6, 4, 4, !K(filled));
      if (abletonGrid->GetButtonState(AbletonDevice::kVolumeEncoderTouch) && abletonGrid->GetButtonState(AbletonDevice::kLoopButton))
         lcd->DrawRect(42, 6, 4, 4, !K(filled));
      if (abletonGrid->GetButtonState(AbletonDevice::kPageLeftButton) ||
          abletonGrid->GetButtonState(AbletonDevice::kPageRightButton))
         lcd->DrawRect(67, 6, 4, 4, !K(filled));
      if (abletonGrid->GetButtonState(AbletonDevice::kClickyEncoderTouch))
         lcd->DrawRect(97, 6, 4, 4, !K(filled));

      int stepsPerBeat = TheTransport->CountInStandardMeasure(mInterval) / TheTransport->GetTimeSigBottom();

      for (const auto* element : mCurrentEditElements)
      {
         lcd->DrawLCDText(NoteName(element->GetPitch(), false, true).c_str(), 10, y, LCDFONT_STYLE_REGULAR);
         lcd->DrawLCDText(ofToString(element->GetVelocity(), 2).c_str(), 35, y, LCDFONT_STYLE_REGULAR);
         lcd->DrawLCDText(ofToString((element->mCol + element->mOffset) / stepsPerBeat, 3).c_str(), 60, y, LCDFONT_STYLE_REGULAR);
         lcd->DrawLCDText(ofToString(element->mLength / stepsPerBeat, 3).c_str(), 90, y, LCDFONT_STYLE_REGULAR);
         y += AbletonMoveLCD::kTextLineSpacing;
      }
      return true;
   }

   if (abletonGrid->GetButtonState(AbletonDevice::kLoopButton))
   {
      lcd->DrawLCDText(("num measures: " + ofToString(mNumMeasures)).c_str(), 10, 13, LCDFONT_STYLE_REGULAR);
      lcd->DrawLCDText(GetCurrentEditMeasureString().c_str(), 10, 13 + AbletonMoveLCD::kTextLineSpacing, LCDFONT_STYLE_REGULAR);
      if (abletonGrid->GetButtonState(AbletonDevice::kVolumeEncoderTouch))
         lcd->DrawRect(10 + 20 + mEditMeasureOffsetSlider * 20, 13, 1, 5, K(filled));
      return true;
   }
   return false;
}

void NoteCanvas::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();
   ofFill();
   for (int i = 0; i < 128; ++i)
   {
      int pitch = 127 - i;
      if (pitch % TheScale->GetPitchesPerOctave() == TheScale->ScaleRoot() % TheScale->GetPitchesPerOctave())
         mCanvas->SetRowColor(i, ofColor(0, 255, 0, 80));
      else if (pitch % TheScale->GetPitchesPerOctave() == (TheScale->ScaleRoot() + 7) % TheScale->GetPitchesPerOctave())
         mCanvas->SetRowColor(i, ofColor(200, 150, 0, 80));
      else if (TheScale->IsInScale(pitch))
         mCanvas->SetRowColor(i, ofColor(100, 75, 0, 80));
      else
         mCanvas->SetRowColor(i, ofColor(100, 100, 100, 30));
   }
   ofPopStyle();

   mCanvas->SetCursorPos(GetCurPos(gTime));

   mCanvas->Draw();
   mCanvasTimeline->Draw();
   mCanvasScrollbarHorizontal->Draw();
   mCanvasScrollbarVertical->Draw();

   ofPushStyle();
   ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
   for (int i = 0; i < mCanvas->GetNumVisibleRows(); ++i)
   {
      int pitch = 127 - mCanvas->GetRowOffset() - i;
      float boxHeight = (float(mCanvas->GetHeight()) / mCanvas->GetNumVisibleRows());
      float y = mCanvas->GetPosition(true).y + i * boxHeight;
      float scale = MIN(boxHeight - 2, 18);
      DrawTextNormal(NoteName(pitch, false, true) + "(" + ofToString(pitch) + ")", mCanvas->GetPosition(true).x + 2, y - (scale / 8) + boxHeight, scale);
   }
   ofPopStyle();

   if (mShowIntervals)
   {
      ofPushMatrix();
      ofTranslate(mCanvas->GetPosition(true).x, mCanvas->GetPosition(true).y);
      ofPushStyle();
      ofSetLineWidth(3);
      auto elements = mCanvas->GetElements();
      for (auto e1 : elements)
      {
         for (auto e2 : elements)
         {
            if (e1 != e2)
            {
               if (abs(e1->GetStart() - e2->GetStart()) < (1.0 / 32) / mNumMeasures)
               {
                  int interval = abs(e1->mRow - e2->mRow);
                  if (interval >= 3 && interval <= 7)
                  {
                     auto rect1 = e1->GetRect(true, false);
                     auto rect2 = e2->GetRect(true, false);
                     float offset = 0;
                     if (interval == 3)
                     {
                        ofSetColor(255, 0, 0, 50);
                     }
                     if (interval == 4)
                     {
                        ofSetColor(0, 255, 0, 50);
                     }
                     if (interval == 5)
                     {
                        ofSetColor(0, 0, 255, 50);
                        offset = -1;
                     }
                     if (interval == 6)
                     {
                        ofSetColor(255, 0, 255, 50);
                        offset = 3;
                     }
                     if (interval == 7)
                     {
                        ofSetColor(0, 0, 0, 50);
                        offset = -3;
                     }

                     ofLine(rect1.x + offset, rect1.getCenter().y, rect2.x + offset, rect2.getCenter().y);
                     ofLine(rect1.x + offset, rect1.getCenter().y, rect1.x + 5 + offset, rect1.getCenter().y);
                     ofLine(rect2.x + offset, rect2.getCenter().y, rect2.x + 5 + offset, rect2.getCenter().y);
                  }
               }
            }
         }
      }
      ofPopStyle();
      ofPopMatrix();
   }

   mCanvasControls->Draw();
   mQuantizeButton->Draw();
   mSaveMidiButton->Draw();
   mLoadMidiButton->Draw();
   mLoadMidiTrackEntry->Draw();
   //mClipButton->Draw();
   mPlayCheckbox->Draw();
   mRecordCheckbox->Draw();
   mFreeRecordCheckbox->Draw();
   mNumMeasuresSlider->Draw();
   mIntervalSelector->Draw();
   mShowIntervalsCheckbox->Draw();

   if (mRecord)
   {
      ofPushStyle();
      ofSetColor(205 + 50 * (cosf(TheTransport->GetMeasurePos(gTime) * 4 * FTWO_PI)), 0, 0);
      ofSetLineWidth(4);
      ofRect(mCanvas->GetPosition(true).x, mCanvas->GetPosition(true).y, mCanvas->GetWidth(), mCanvas->GetHeight());
      ofPopStyle();
   }
}

void NoteCanvas::DoubleLoop()
{
   //crop contents to current length
   std::list<CanvasElement*> toRemove;
   for (auto* element : mCanvas->GetElements())
   {
      NoteCanvasElement* noteElement = static_cast<NoteCanvasElement*>(element);
      if (noteElement->mCol >= mCanvas->GetNumCols())
         toRemove.push_back(element);
   }
   for (auto* remove : toRemove)
      mCanvas->RemoveElement(remove);

   //duplicate current contents and shift
   std::list<NoteCanvasElement*> newElements;
   for (auto* element : mCanvas->GetElements())
   {
      auto* newNote = static_cast<NoteCanvasElement*>(element->CreateDuplicate());
      newNote->mCol += mCanvas->GetNumCols();
      newElements.push_back(newNote);
   }

   SetNumMeasures(mNumMeasures * 2);

   for (auto* element : newElements)
   {
      mCanvas->AddElement(element);
   }
}

void NoteCanvas::CopyNotesToClipboard(int stepIndex)
{
   mClipboardElements.clear();
   for (auto* element : mCurrentEditElements)
      mClipboardElements.push_back(element);
   int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop() / TheTransport->GetTimeSigBottom();
   mClipboardCopyFromStep = stepIndex + mEditMeasureOffset * stepsPerMeasure;
}

namespace
{
   const float extraW = 20;
   const float extraH = 163;
}

void NoteCanvas::Resize(float w, float h)
{
   w = MAX(w, 390);
   h = MAX(h, 200);
   mWidth = w;
   mHeight = h;
   mCanvas->SetDimensions(w - extraW, h - extraH);
}

void NoteCanvas::SetNumMeasures(int numMeasures)
{
   mNumMeasures = numMeasures;
   mCanvas->SetLength(mNumMeasures);
   mCanvas->SetNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
   if (mInterval < kInterval_8n)
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval));
   else
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval) / 4);
   mCanvas->mViewStart = 0;
   mCanvas->mViewEnd = mNumMeasures;
   mCanvas->mLoopStart = 0;
   mCanvas->mLoopEnd = mNumMeasures;
}

void NoteCanvas::SetRecording(bool rec)
{
   mRecord = rec;

   if (mRecord)
      mPlay = true;

   for (int pitch = 0; pitch < 128; ++pitch)
      mInputNotes[pitch] = nullptr;
}

void NoteCanvas::ClipNotes()
{
   bool anyHighlighted = false;
   float earliest = FLT_MAX;
   float latest = 0;
   std::vector<CanvasElement*> toDelete;
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
      {
         anyHighlighted = true;
         if (element->GetStart() < earliest)
            earliest = element->GetStart();
         if (element->GetEnd() > latest)
            latest = element->GetEnd();
      }
      else
      {
         toDelete.push_back(element);
      }
   }

   if (anyHighlighted)
   {
      for (auto* remove : toDelete)
         mCanvas->RemoveElement(remove);

      int earliestMeasure = int(earliest * mNumMeasures);
      int latestMeasure = int(latest * mNumMeasures) + 1;
      int clipStart = 0;
      int clipEnd = mNumMeasures;

      while (earliestMeasure - clipStart >= (clipEnd - clipStart) / 2 ||
             clipEnd - latestMeasure >= (clipEnd - clipStart) / 2)
      {
         if (earliestMeasure - clipStart >= (clipEnd - clipStart) / 2)
            clipStart += (clipEnd - clipStart) / 2;
         if (clipEnd - latestMeasure >= (clipEnd - clipStart) / 2)
            clipEnd -= (clipEnd - clipStart) / 2;
      }

      SetNumMeasures(clipEnd - clipStart);

      ofLog() << earliest << " " << latest << " " << clipStart << " " << clipEnd;

      int shift = -clipStart;

      for (auto* element : mCanvas->GetElements())
         element->SetStart(element->GetStart() + float(shift) / mNumMeasures, true);
   }
}

void NoteCanvas::QuantizeNotes()
{
   bool anyHighlighted = false;
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetHighlighted())
      {
         anyHighlighted = true;
         break;
      }
   }
   for (auto* element : mCanvas->GetElements())
   {
      if (anyHighlighted == false || element->GetHighlighted())
      {
         element->mCol = int(element->mCol + element->mOffset + .5f) % mCanvas->GetNumCols();
         element->mOffset = 0;
      }
   }
}

void NoteCanvas::LoadMidi()
{
   using namespace juce;
   String file_pattern = "*.mid;*.midi";
   if (File::areFileNamesCaseSensitive())
      file_pattern += ";" + file_pattern.toUpperCase();
   FileChooser chooser("Load midi", File(ofToDataPath("")), file_pattern, true, false, TheSynth->GetFileChooserParent());
   if (chooser.browseForFileToOpen())
   {
      bool wasPlaying = mPlay;
      mPlay = false;

      Clear(NextBufferTime(false));
      SetNumMeasures(1);
      File file = chooser.getResult();
      FileInputStream inputStream(file);
      MidiFile midifile;
      if (midifile.readFrom(inputStream))
      {
         int ticksPerQuarterNote = midifile.getTimeFormat();
         int trackToGet = 0;
         if (midifile.getNumTracks() > 1)
            trackToGet = mLoadMidiTrack;
         const MidiMessageSequence* trackSequence = midifile.getTrack(trackToGet);
         for (int eventIndex = 0; eventIndex < trackSequence->getNumEvents(); eventIndex++)
         {
            MidiMessageSequence::MidiEventHolder* noteEvent = trackSequence->getEventPointer(eventIndex);
            if (noteEvent->noteOffObject)
            {
               int note = noteEvent->message.getNoteNumber();
               int veloc = noteEvent->message.getVelocity() * 1.27;
               double start = noteEvent->message.getTimeStamp() / ticksPerQuarterNote / TheTransport->CountInStandardMeasure(kInterval_4n);
               double end = noteEvent->noteOffObject->message.getTimeStamp() / ticksPerQuarterNote / TheTransport->CountInStandardMeasure(kInterval_4n);
               double length = end - start;
               AddNote(start, note, veloc, length, -1, ModulationParameters());
            }
         }
         float latest = 0.0;
         for (auto* element : mCanvas->GetElements())
         {
            if (element->GetEnd() > latest)
               latest = element->GetEnd();
         }
         mNumMeasuresSlider->SetExtents(0, static_cast<int>(std::ceil(latest)));
         FitNotes(true, true);
      }

      mPlay = wasPlaying;
   }
}

void NoteCanvas::SaveMidi()
{
   using namespace juce;
   constexpr static int ticksPerQuarterNote = 960;

   FileChooser chooser("Save midi", File(ofToDataPath("midi")), "*.mid", true, false, TheSynth->GetFileChooserParent());
   if (chooser.browseForFileToSave(true))
   {
      MidiFile midifile;
      midifile.setTicksPerQuarterNote(ticksPerQuarterNote);
      MidiMessageSequence track1;
      MidiMessage trackTimeSig = MidiMessage::timeSignatureMetaEvent(TheTransport->GetTimeSigTop(), TheTransport->GetTimeSigBottom());
      track1.addEvent(trackTimeSig);
      for (auto* element : mCanvas->GetElements())
      {
         NoteCanvasElement* noteOnElement = static_cast<NoteCanvasElement*>(element);
         int noteNumber = mCanvas->GetNumRows() - noteOnElement->mRow - 1;
         float noteStart = (element->mCol + element->mOffset) * ticksPerQuarterNote *
                           +TheTransport->GetMeasureFraction(mInterval) / TheTransport->GetMeasureFraction(kInterval_4n);
         float velocity = noteOnElement->GetVelocity();
         MidiMessage messageOn = MidiMessage::noteOn(1, noteNumber, velocity);
         messageOn.setTimeStamp(noteStart);
         track1.addEvent(messageOn);
         float noteEnd = (element->mCol + element->mOffset + element->mLength) * ticksPerQuarterNote *
                         +TheTransport->GetMeasureFraction(mInterval) / TheTransport->GetMeasureFraction(kInterval_4n);
         MidiMessage messageOff = MidiMessage::noteOff(1, noteNumber, velocity);
         messageOff.setTimeStamp(noteEnd);
         track1.addEvent(messageOff);
      }
      midifile.addTrack(track1);
      std::string savePath = chooser.getResult().getFullPathName().toStdString();
      File f(savePath);
      FileOutputStream out(f);
      midifile.writeTo(out);
   }
}

void NoteCanvas::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int pitch = 0; pitch < 128; ++pitch)
         mInputNotes[pitch] = nullptr;
      mNoteOutput.Flush(time);
   }
   if (checkbox == mPlayCheckbox)
   {
      if (!mPlay)
      {
         mRecord = false;
         mStopQueued = true;
      }
   }
   if (checkbox == mRecordCheckbox)
   {
      SetRecording(mRecord);
   }
   if (checkbox == mFreeRecordCheckbox)
   {
      if (mFreeRecord)
      {
         SetRecording(true);
         mFreeRecordStartMeasure = -1;
      }
   }
}

void NoteCanvas::ButtonClicked(ClickButton* button, double time)
{
   if (button == mQuantizeButton)
      QuantizeNotes();

   if (button == mClipButton)
      ClipNotes();

   if (button == mLoadMidiButton)
      LoadMidi();

   if (button == mSaveMidiButton)
      SaveMidi();
}

void NoteCanvas::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void NoteCanvas::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumMeasuresSlider)
   {
      SetNumMeasures(mNumMeasures);
   }
}

void NoteCanvas::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      UpdateNumColumns();
   }
}

void NoteCanvas::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("canvaswidth", moduleInfo, 390, 390, 99999, K(isTextField));
   mModuleSaveData.LoadFloat("canvasheight", moduleInfo, 200, 40, 99999, K(isTextField));
   mModuleSaveData.LoadString("grid_keyboard_interface", moduleInfo, "", FillDropdown<LaunchpadKeyboard*>);

   SetUpFromSaveData();
}

void NoteCanvas::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mCanvas->SetDimensions(mModuleSaveData.GetFloat("canvaswidth"), mModuleSaveData.GetFloat("canvasheight"));
   IDrawableModule* gridKeyboardInterface = TheSynth->FindModule(mModuleSaveData.GetString("grid_keyboard_interface"), false);
   mGridKeyboardInterface = dynamic_cast<LaunchpadKeyboard*>(gridKeyboardInterface);
}

void NoteCanvas::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["canvaswidth"] = mCanvas->GetWidth();
   moduleInfo["canvasheight"] = mCanvas->GetHeight();
   moduleInfo["grid_keyboard_interface"] = mGridKeyboardInterface ? mGridKeyboardInterface->Path() : "";
}

void NoteCanvas::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mCanvas->SaveState(out);
}

void NoteCanvas::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   mCanvas->LoadState(in);
}