//
//  NoteLooper.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/31/13.
//
//

#include "NoteLooper.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "MidiController.h"
#include "ModularSynth.h"
#include "Profiler.h"

NoteLooper::NoteLooper()
: mWidth(305)
, mHeight(140)
, mMinRow(127)
, mMaxRow(0)
, mWrite(false)
, mWriteCheckbox(nullptr)
, mNumMeasures(1)
, mNumMeasuresSlider(nullptr)
, mDeleteOrMute(false)
, mDeleteOrMuteCheckbox(nullptr)
, mVoiceRoundRobin(kNumVoices-1)
{
   TheTransport->AddAudioPoller(this);
}

void NoteLooper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mWriteCheckbox = new Checkbox(this,"write",4,4,&mWrite);
   mDeleteOrMuteCheckbox = new Checkbox(this, "del/mute", 4, 24, &mDeleteOrMute);
   mNumMeasuresSlider = new IntSlider(this,"num bars",80,4,85,15,&mNumMeasures,1,8);   
   mClearButton = new ClickButton(this, "clear", 80, 24);

   mCanvas = new Canvas(this, 3, 45, mWidth-6, mHeight-48, L(length, 1), L(rows, 128), L(cols, 16), &(NoteCanvasElement::Create));
   AddUIControl(mCanvas);
   mCanvas->SetNumVisibleRows(1);
   mCanvas->SetRowOffset(0);
   mCanvas->SetScrollable(false);
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

   if (mMinRow <= mMaxRow)
   {
      mCanvas->SetRowOffset(mMinRow);
      mCanvas->SetNumVisibleRows(mMaxRow - mMinRow + 1);
   }
   mCanvas->SetCursorPos(GetCurPos(gTime));
   mCanvas->Draw();
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

   double cursorPlayTime = gTime;
   if (Transport::sDoEventLookahead)
      cursorPlayTime += Transport::sEventEarlyMs;
   else
      cursorPlayTime += amount * TheTransport->MsPerBar();
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
            mNoteOutput.PlayNote(time, pitch, 0, mCurrentNotes[pitch]->GetVoiceIdx());
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
            mNoteOutput.PlayNote(time, pitch, note->GetVelocity() * 127, note->GetVoiceIdx(), ModulationParameters(note->GetPitchBend(), note->GetModWheel(), note->GetPressure(), note->GetPan()));
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
         float bend = 0;
         float mod = 0;
         float pressure = 0;
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

void NoteLooper::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (voiceIdx != -1)  //try to pick a voice that is unique, to avoid stomping on the voices of already-recorded notes when overdubbing
   {
      if (velocity > 0)
         voiceIdx = GetNewVoice(voiceIdx);
      else
         voiceIdx = mVoiceMap[voiceIdx];
   }

   mNoteOutput.PlayNote(time, pitch, velocity, voiceIdx, modulation);

   if ((mEnabled && mWrite) || (mInputNotes[pitch] && velocity == 0))
   {
      if (mInputNotes[pitch]) //handle note-offs or retriggers
      {
         double endPos = GetCurPos(time);
         if (mInputNotes[pitch]->GetStart() > endPos)
            endPos += 1; //wrap
         mInputNotes[pitch]->SetEnd(endPos);
         mInputNotes[pitch] = nullptr;
      }

      if (velocity > 0)
      {
         double measurePos = GetCurPos(time) * mNumMeasures;
         NoteCanvasElement* element = AddNote(measurePos, pitch, velocity, 1 / mCanvas->GetNumCols(), voiceIdx, modulation);
         mInputNotes[pitch] = element;
      }
   }
}

NoteCanvasElement* NoteLooper::AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx/*=-1*/, ModulationParameters modulation/* = ModulationParameters()*/)
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

      const int kMinVoiceNumber = 2;   //MPE synths seem to reserve channels <2 for global params
      mVoiceRoundRobin = ((mVoiceRoundRobin - kMinVoiceNumber) + 1) % (kNumVoices - kMinVoiceNumber) + kMinVoiceNumber; //wrap around a 2-15 range
      mVoiceMap[voiceIdx] = ret;
   }
   return ret;
}

void NoteLooper::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (size_t i = 0; i < mCurrentNotes.size(); ++i)
      {
         if (mCurrentNotes[i] != nullptr)
         {
            mNoteOutput.PlayNote(gTime, i, 0, mCurrentNotes[i]->GetVoiceIdx());
            mCurrentNotes[i] = nullptr;
         }
      }
   }
}

void NoteLooper::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteLooper::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumMeasuresSlider)
      SetNumMeasures(mNumMeasures);
}

void NoteLooper::ButtonClicked(ClickButton* button)
{
   if (button == mClearButton)
   {
      for (size_t i = 0; i < mCurrentNotes.size(); ++i)
      {
         if (mCurrentNotes[i] != nullptr)
         {
            mNoteOutput.PlayNote(gTime, i, 0, mCurrentNotes[i]->GetVoiceIdx());
            mCurrentNotes[i] = nullptr;
         }
      }
      for (size_t i = 0; i < mInputNotes.size(); ++i)
      {
         if (mInputNotes[i] != nullptr)
         {
            mNoteOutput.PlayNote(gTime, i, 0, mInputNotes[i]->GetVoiceIdx());
            mInputNotes[i] = nullptr;
         }
      }
      mCanvas->Clear();
   }
}

void NoteLooper::SetNumMeasures(int numMeasures)
{
   mNumMeasures = numMeasures;
   mCanvas->SetLength(mNumMeasures);
   mCanvas->SetNumCols(TheTransport->CountInStandardMeasure(kInterval_8n) * mNumMeasures);
   mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(kInterval_8n));
   mCanvas->mStart = 0;
   mCanvas->mEnd = mNumMeasures;
}

void NoteLooper::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void NoteLooper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteLooper::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


