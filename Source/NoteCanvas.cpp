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

#include "juce_gui_basics/juce_gui_basics.h"

NoteCanvas::NoteCanvas()
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
      mCanvas->SetRowOffset(element->mRow - mCanvas->GetNumVisibleRows() / 2);
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

void NoteCanvas::FitNotes()
{
   float latest = 0.0;
   for (auto* element : mCanvas->GetElements())
   {
      if (element->GetEnd() > latest)
         latest = element->GetEnd();
   }
   SetNumMeasures(static_cast<int>(std::ceil(latest)));
}

void NoteCanvas::CanvasUpdated(Canvas* canvas)
{
   if (canvas == mCanvas)
   {
   }
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

namespace
{
   const float extraW = 20;
   const float extraH = 163;
}

void NoteCanvas::Resize(float w, float h)
{
   w = MAX(w - extraW, 390);
   h = MAX(h - extraH, 40);
   mCanvas->SetDimensions(w, h);
}

void NoteCanvas::GetModuleDimensions(float& width, float& height)
{
   width = mCanvas->GetWidth() + extraW;
   height = mCanvas->GetHeight() + extraH;
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
         midifile.convertTimestampTicksToSeconds();
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
         FitNotes();
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

   SetUpFromSaveData();
}

void NoteCanvas::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mCanvas->SetDimensions(mModuleSaveData.GetFloat("canvaswidth"), mModuleSaveData.GetFloat("canvasheight"));
}

void NoteCanvas::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["canvaswidth"] = mCanvas->GetWidth();
   moduleInfo["canvasheight"] = mCanvas->GetHeight();
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