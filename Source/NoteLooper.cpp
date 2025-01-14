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
//  NoteLooper.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/31/13.
//
//

#include "NoteLooper.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "MidiController.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"

NoteLooper::NoteLooper()
{
}

void NoteLooper::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void NoteLooper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   float w, h;
   UIBLOCK(80);
   CHECKBOX(mWriteCheckbox, "write", &mWrite);
   CHECKBOX(mDeleteOrMuteCheckbox, "del/mute", &mDeleteOrMute);
   UIBLOCK_NEWCOLUMN();
   INTSLIDER(mNumMeasuresSlider, "num bars", &mNumMeasures, 1, 8);
   BUTTON(mClearButton, "clear");
   ENDUIBLOCK(w, h);

   UIBLOCK(w + 10, 3, 45);
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      BUTTON(mSavedPatterns[i].mStoreButton, ("store" + ofToString(i)).c_str());
      BUTTON(mSavedPatterns[i].mLoadButton, ("load" + ofToString(i)).c_str());
      UIBLOCK_NEWCOLUMN();
   }
   ENDUIBLOCK0();

   mCanvas = new Canvas(this, 3, 45, mWidth - 6, mHeight - 48, L(length, 1), L(rows, 128), L(cols, 16), &(NoteCanvasElement::Create));
   AddUIControl(mCanvas);
   mCanvas->SetNumVisibleRows(1);
   mCanvas->SetRowOffset(0);
   SetNumMeasures(mNumMeasures);
}

NoteLooper::~NoteLooper()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteLooper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mWriteCheckbox->Draw();
   mNumMeasuresSlider->Draw();
   mDeleteOrMuteCheckbox->Draw();
   mClearButton->Draw();
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      mSavedPatterns[i].mStoreButton->Draw();
      mSavedPatterns[i].mLoadButton->Draw();
      if (mSavedPatterns[i].mNotes.size() > 0)
      {
         ofPushStyle();
         ofFill();
         ofSetColor(0, 255, 0, 80);
         ofRectangle rect = mSavedPatterns[i].mLoadButton->GetRect(K(local));
         ofRect(rect);
         ofPopStyle();
      }
   }

   if (mMinRow <= mMaxRow)
   {
      mCanvas->SetRowOffset(mMinRow);
      mCanvas->SetNumVisibleRows(mMaxRow - mMinRow + 1);
   }
   mCanvas->SetCursorPos(GetCurPos(gTime));
   mCanvas->Draw();
}

bool NoteLooper::DrawToPush2Screen()
{
   ofRectangle rect = mCanvas->GetRect(true);

   mCanvas->SetPosition(125, 3);
   mCanvas->SetDimensions(600, 40);

   mCanvas->SetCursorPos(GetCurPos(gTime));
   mCanvas->Draw();

   mCanvas->SetPosition(rect.x, rect.y);
   mCanvas->SetDimensions(rect.width, rect.height);

   return false;
}

void NoteLooper::Resize(float w, float h)
{
   mWidth = MAX(w, 370);
   mHeight = MAX(h, 140);
   mCanvas->SetDimensions(mWidth - 6, mHeight - 48);
}

double NoteLooper::GetCurPos(double time) const
{
   return ((TheTransport->GetMeasure(time) % mNumMeasures) + TheTransport->GetMeasurePos(time)) / mNumMeasures;
}

void NoteLooper::OnTransportAdvanced(float amount)
{
   PROFILER(NoteLooper);

   if (!mEnabled)
   {
      mCanvas->SetCursorPos(-1);
      return;
   }

   double cursorPlayTime = NextBufferTime(mAllowLookahead);
   double curPos = GetCurPos(cursorPlayTime);

   if (mDeleteOrMute)
   {
      if (mWrite)
         mCanvas->EraseElementsAt(curPos);
   }
   else
   {
      mCanvas->FillElementsAt(curPos, mNoteChecker);
   }

   for (int i = 0; i < 128; ++i)
   {
      int pitch = 128 - i - 1;
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
      if (nowOn && mInputNotes[pitch] == nullptr && hasChanged)
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

void NoteLooper::PlayNote(NoteMessage note)
{
   if (note.voiceIdx != -1) //try to pick a voice that is unique, to avoid stomping on the voices of already-recorded notes when overdubbing
   {
      if (note.velocity > 0)
         note.voiceIdx = GetNewVoice(note.voiceIdx);
      else
         note.voiceIdx = mVoiceMap[note.voiceIdx];
   }

   mNoteOutput.PlayNote(note);

   if ((mEnabled && mWrite) || (mInputNotes[note.pitch] && note.velocity == 0))
   {
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
         double measurePos = GetCurPos(note.time) * mNumMeasures;
         NoteCanvasElement* element = AddNote(measurePos, note.pitch, note.velocity, 1 / mCanvas->GetNumCols(), note.voiceIdx, note.modulation);
         mInputNotes[note.pitch] = element;
      }
   }
}

NoteCanvasElement* NoteLooper::AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx /*=-1*/, ModulationParameters modulation /* = ModulationParameters()*/)
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

   if (row < mMinRow)
      mMinRow = row;
   if (row > mMaxRow)
      mMaxRow = row;

   return element;
}

int NoteLooper::GetNewVoice(int voiceIdx)
{
   int ret = voiceIdx;
   if (voiceIdx >= 0 && voiceIdx < kNumVoices)
   {
      //TODO(Ryan) do a round robin for now, maybe in the future do something smarter like looking at what voices are already recorded and pick an unused one
      ret = mVoiceRoundRobin;

      const int kMinVoiceNumber = 2; //MPE synths seem to reserve channels <2 for global params
      mVoiceRoundRobin = ((mVoiceRoundRobin - kMinVoiceNumber) + 1) % (kNumVoices - kMinVoiceNumber) + kMinVoiceNumber; //wrap around a 2-15 range
      mVoiceMap[voiceIdx] = ret;
   }
   return ret;
}

void NoteLooper::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < (int)mCurrentNotes.size(); ++i)
      {
         if (mCurrentNotes[i] != nullptr)
         {
            mNoteOutput.PlayNote(NoteMessage(time, i, 0, mCurrentNotes[i]->GetVoiceIdx()));
            mCurrentNotes[i] = nullptr;
         }
      }
   }
}

void NoteLooper::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void NoteLooper::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumMeasuresSlider)
      SetNumMeasures(mNumMeasures);
}

void NoteLooper::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
   {
      for (int i = 0; i < (int)mCurrentNotes.size(); ++i)
      {
         if (mCurrentNotes[i] != nullptr)
         {
            mNoteOutput.PlayNote(NoteMessage(time, i, 0, mCurrentNotes[i]->GetVoiceIdx()));
            mCurrentNotes[i] = nullptr;
         }
      }
      for (int i = 0; i < (int)mInputNotes.size(); ++i)
      {
         if (mInputNotes[i] != nullptr)
         {
            mNoteOutput.PlayNote(NoteMessage(time, i, 0, mInputNotes[i]->GetVoiceIdx()));
            mInputNotes[i] = nullptr;
         }
      }
      mCanvas->Clear();
      mMinRow = 127;
      mMaxRow = 0;
   }
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      if (button == mSavedPatterns[i].mStoreButton)
         mSavedPatterns[i].mNotes = mCanvas->GetElements();

      if (button == mSavedPatterns[i].mLoadButton)
      {
         mCanvas->Clear();
         mMinRow = 127;
         mMaxRow = 0;
         for (auto& element : mSavedPatterns[i].mNotes)
         {
            mCanvas->AddElement(element);

            int row = element->mRow;
            if (row < mMinRow)
               mMinRow = row;
            if (row > mMaxRow)
               mMaxRow = row;
         }
      }
   }
}

void NoteLooper::SetNumMeasures(int numMeasures)
{
   mNumMeasures = numMeasures;
   mCanvas->SetLength(mNumMeasures);
   mCanvas->SetNumCols(TheTransport->CountInStandardMeasure(kInterval_8n) * mNumMeasures);
   mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(kInterval_8n));
   mCanvas->mViewStart = 0;
   mCanvas->mViewEnd = mNumMeasures;
   mCanvas->mLoopStart = 0;
   mCanvas->mLoopEnd = mNumMeasures;
}

void NoteLooper::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void NoteLooper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("allow_lookahead", moduleInfo, true);

   SetUpFromSaveData();
}

void NoteLooper::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mAllowLookahead = mModuleSaveData.GetBool("allow_lookahead");
}

void NoteLooper::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWidth;
   out << mHeight;

   out << mMinRow;
   out << mMaxRow;

   out << (int)mSavedPatterns.size();
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      out << (int)mSavedPatterns[i].mNotes.size();
      for (auto& note : mSavedPatterns[i].mNotes)
      {
         out << note->mCol;
         out << note->mRow;
         note->SaveState(out);
      }
   }
}

void NoteLooper::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWidth;
   in >> mHeight;
   Resize(mWidth, mHeight);

   in >> mMinRow;
   in >> mMaxRow;

   int numPatterns;
   in >> numPatterns;
   LoadStateValidate(numPatterns == mSavedPatterns.size());
   for (size_t i = 0; i < mSavedPatterns.size(); ++i)
   {
      int numNotes;
      in >> numNotes;
      mSavedPatterns[i].mNotes.resize(numNotes);
      for (int j = 0; j < numNotes; ++j)
      {
         int col, row;
         in >> col;
         in >> row;
         mSavedPatterns[i].mNotes[j] = NoteCanvasElement::Create(mCanvas, col, row);
         mSavedPatterns[i].mNotes[j]->LoadState(in);
      }
   }
}
