/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  ChordKeyboard.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/18/26.
//
//

#include "ChordKeyboard.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

#include <cstring>

ChordKeyboard::ChordKeyboard()
{
   mTransportPriority = ITimeListener::kTransportPriorityEarly;
}

ChordKeyboard::~ChordKeyboard()
{
   TheTransport->RemoveAudioPoller(this);
}

void ChordKeyboard::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
   TheTransport->AddListener(this, mQuantizeInterval, OffsetInfo(0, true), true);
}

void ChordKeyboard::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mScaleModeCheckbox, "scale mode", &mScaleMode);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mQuantizeIntervalSelector, "quantize", (int*)&mQuantizeInterval, 50);
   UIBLOCK_NEWLINE();
   CHECKBOX(mDimCheckbox, "dim", &mChordSettings.mDim);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mMinorCheckbox, "minor", &mChordSettings.mMinor);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mMajorCheckbox, "major", &mChordSettings.mMajor);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mSusCheckbox, "sus", &mChordSettings.mSus);
   UIBLOCK_NEWLINE();
   CHECKBOX(mSixCheckbox, "six", &mChordSettings.mSix);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mMin7Checkbox, "min7", &mChordSettings.mMin7);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mMaj7Checkbox, "maj7", &mChordSettings.mMaj7);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mNineCheckbox, "nine", &mChordSettings.mNine);
   UIBLOCK_NEWLINE();
   DROPDOWN(mPlayOptionsSelector, "play options", (int*)&mChordSettings.mPlayOptions, 100);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLatchKeyCheckbox, "latch keyboard", &mLatchKey);
   ENDUIBLOCK(mWidth, mHeight);

   UIBLOCK(mWidth + 3, 3);
   UIBLOCK_PUSHSLIDERWIDTH(150);
   INTSLIDER(mVoicingSlider, "voicing", &mVoicing, 0, 128 - 12 - 1);
   INTSLIDER(mBassVoicingSlider, "bass voicing", &mBassVoicing, 0, 128 - 12 - 1);
   INTSLIDER(mVelocityOverrideSlider, "velocity override", &mVelocityOverride, -1, 127);
   DROPDOWN(mChordStyleSelector, "chord style", (int*)&mChordStyle, 100);
   ENDUIBLOCK(mWidth, mHeight);

   mHeight += 100;

   mBassCable = new AdditionalNoteCable();
   mBassCable->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
   mBassCable->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kBottom);
   AddPatchCableSource(mBassCable->GetPatchCableSource());
   mBassCable->GetPatchCableSource()->SetManualPosition(mWidth, mHeight - 70);

   mKeyboardDrawOptions.mWidth = mWidth;
   mKeyboardDrawOptions.mHeight = 50;
   mKeyboardDrawOptions.mNumOctaves = 7;
   mKeyboardDrawOptions.mRootOctave = 2;
   mKeyboardDrawOptions.mHideLabels = true;
   mKeyboardDrawOptions.mShowScale = false;
   mKeyboardDrawOptions.mEnabled = IsEnabled();

   mQuantizeIntervalSelector->AddLabel("none", (int)kInterval_None);
   mQuantizeIntervalSelector->AddLabel("32n", (int)kInterval_32n);
   mQuantizeIntervalSelector->AddLabel("16n", (int)kInterval_16n);
   mQuantizeIntervalSelector->AddLabel("8n", (int)kInterval_8n);
   mQuantizeIntervalSelector->AddLabel("4n", (int)kInterval_4n);
   mQuantizeIntervalSelector->DrawLabel(true);

   mPlayOptionsSelector->AddLabel("chord and bass", (int)PlayOptions::ChordAndBass);
   mPlayOptionsSelector->AddLabel("chord only", (int)PlayOptions::ChordOnly);
   mPlayOptionsSelector->AddLabel("bass only", (int)PlayOptions::BassOnly);
   mPlayOptionsSelector->AddLabel("chord on press", (int)PlayOptions::ChordOnPress);
   mPlayOptionsSelector->AddLabel("immediate", (int)PlayOptions::ImmediateChord);
   mPlayOptionsSelector->AddLabel("immediate (chord only)", (int)PlayOptions::ImmediateChordOnly);

   mChordStyleSelector->AddLabel("closed", (int)ChordStyle::Closed);
   mChordStyleSelector->AddLabel("spread 3", (int)ChordStyle::SpreadThird);
   mChordStyleSelector->AddLabel("spread 7", (int)ChordStyle::SpreadSeventh);
   mChordStyleSelector->AddLabel("spread 3&7", (int)ChordStyle::SpreadThirdAndSeventh);
   mChordStyleSelector->AddLabel("doubled 1", (int)ChordStyle::DoubledRoot);
   mChordStyleSelector->AddLabel("doubled 1&3", (int)ChordStyle::DoubledRootAndThird);
   mChordStyleSelector->AddLabel("doubled", (int)ChordStyle::Doubled);
}

void ChordKeyboard::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mScaleModeCheckbox->Draw();
   mQuantizeIntervalSelector->Draw();
   mDimCheckbox->Draw();
   mMajorCheckbox->Draw();
   mMinorCheckbox->Draw();
   mSusCheckbox->Draw();
   mSixCheckbox->Draw();
   mMin7Checkbox->Draw();
   mMaj7Checkbox->Draw();
   mNineCheckbox->Draw();
   mPlayOptionsSelector->Draw();
   mLatchKeyCheckbox->Draw();
   mVoicingSlider->Draw();
   mBassVoicingSlider->Draw();
   mVelocityOverrideSlider->Draw();
   mChordStyleSelector->Draw();

   //cable label
   DrawTextNormal("bass: ", mWidth - 35, mHeight - 70 + 4);

   ofPushStyle();
   ofSetColor(ofColor::white);
   DrawTextBold(GetChordDisplayString(), 3, mHeight - 55, 35);
   ofPopStyle();

   KeyboardDisplay::DrawKeyboard(0, mHeight - 50, mKeyboardDrawOptions, &mLastNoteOnTime, &mLastNoteOffTime);
   bool isBlackKey;
   ofPushStyle();
   ofSetColor(ofColor::white);
   ofSetLineWidth(1);
   ofNoFill();
   int voicingMinX = KeyboardDisplay::GetKeyboardKeyRect(mVoicing, isBlackKey, mKeyboardDrawOptions).x;
   int voicingMaxX = KeyboardDisplay::GetKeyboardKeyRect(mVoicing + 12, isBlackKey, mKeyboardDrawOptions).getMaxX();
   ofRect(0 + voicingMinX, mHeight - 50, voicingMaxX - voicingMinX, 50);
   int bassVoicingMinX = KeyboardDisplay::GetKeyboardKeyRect(mBassVoicing, isBlackKey, mKeyboardDrawOptions).x;
   int bassVoicingMaxX = KeyboardDisplay::GetKeyboardKeyRect(mBassVoicing + 12, isBlackKey, mKeyboardDrawOptions).getMaxX();
   ofSetColor(ofColor::blue);
   ofSetLineWidth(2);
   ofLine(bassVoicingMinX, mHeight, bassVoicingMaxX, mHeight);
   ofSetLineWidth(1);
   if (mBassOutputPitch != -1)
   {
      ofFill();
      ofSetColor(ofColor::blue, 100);
      ofRectangle bassKeyRect = KeyboardDisplay::GetKeyboardKeyRect(mBassOutputPitch, isBlackKey, mKeyboardDrawOptions);
      bassKeyRect.y += mHeight - 50;
      ofRect(bassKeyRect);
   }
   ofPopStyle();
}

void ChordKeyboard::OnTransportAdvanced(float amount)
{
   if (mQuantizeInterval == kInterval_None)
      UpdateOutputNotes(gTime);
}

void ChordKeyboard::OnTimeEvent(double time)
{
   UpdateOutputNotes(time);
}

void ChordKeyboard::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush(time);
   }
   if (checkbox == mDimCheckbox)
   {
      mChordSettings.SetDim(mChordSettings.mDim);
      OnChordSettingsUpdated(time);
   }
   if (checkbox == mMinorCheckbox)
   {
      mChordSettings.SetMinor(mChordSettings.mMinor);
      OnChordSettingsUpdated(time);
   }
   if (checkbox == mMajorCheckbox)
   {
      mChordSettings.SetMajor(mChordSettings.mMajor);
      OnChordSettingsUpdated(time);
   }
   if (checkbox == mSusCheckbox)
   {
      mChordSettings.SetSus(mChordSettings.mSus);
      OnChordSettingsUpdated(time);
   }
   if (checkbox == mSixCheckbox || checkbox == mMin7Checkbox || checkbox == mMajorCheckbox || checkbox == mNineCheckbox)
      OnChordSettingsUpdated(time);
}

void ChordKeyboard::DropdownUpdated(DropdownList* dropdown, int oldVal, double time)
{
   if (dropdown == mQuantizeIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mQuantizeInterval;
   }
}

void ChordKeyboard::OnChordSettingsUpdated(double time)
{
   if (time < mLastKeyPressTime + 100) // changed chord very shortly after playing new pitch
      memcpy(&mLastPlayedChordSettings, &mChordSettings, sizeof(mChordSettings));
}

std::list<int> ChordKeyboard::GetChordPitches(int forPitch)
{
   std::list<int> chordPitches;
   chordPitches.push_back(forPitch);
   int thirdIndex = -1;
   int seventhIndex = -1;
   if (mScaleMode && !GetChordSettings().IsChordButtonPressed()) //always play triad in scale mode, if no other manual chord is selected
   {
      chordPitches.push_back(forPitch + 4);
      thirdIndex = (int)chordPitches.size() - 1;
      chordPitches.push_back(forPitch + 7);
   }
   if (GetChordSettings().mDim)
   {
      chordPitches.push_back(forPitch + 3);
      thirdIndex = (int)chordPitches.size() - 1;
      chordPitches.push_back(forPitch + 6);
   }
   if (GetChordSettings().mMinor)
   {
      chordPitches.push_back(forPitch + 3);
      thirdIndex = (int)chordPitches.size() - 1;
      chordPitches.push_back(forPitch + 7);
   }
   if (GetChordSettings().mMajor)
   {
      chordPitches.push_back(forPitch + 4);
      thirdIndex = (int)chordPitches.size() - 1;
      chordPitches.push_back(forPitch + 7);
   }
   if (GetChordSettings().mSus)
   {
      chordPitches.push_back(forPitch + 5);
      chordPitches.push_back(forPitch + 7);
   }
   if (GetChordSettings().mSix)
   {
      chordPitches.push_back(forPitch + 9);
   }
   if (GetChordSettings().mMin7)
   {
      chordPitches.push_back(forPitch + 10);
      seventhIndex = (int)chordPitches.size() - 1;
   }
   if (GetChordSettings().mMaj7)
   {
      chordPitches.push_back(forPitch + 11);
      seventhIndex = (int)chordPitches.size() - 1;
   }
   if (GetChordSettings().mNine)
   {
      chordPitches.push_back(forPitch + 14);
   }

   for (int& pitch : chordPitches)
      pitch = AdjustForVoicing(pitch, mVoicing);

   int numOriginalPitches = (int)chordPitches.size();
   auto iter = chordPitches.begin();
   for (int i = 0; i < numOriginalPitches; ++i)
   {
      if (i == thirdIndex && (mChordStyle == ChordStyle::SpreadThird || mChordStyle == ChordStyle::SpreadThirdAndSeventh))
         *iter += TheScale->GetPitchesPerOctave();
      if (i == seventhIndex && (mChordStyle == ChordStyle::SpreadSeventh || mChordStyle == ChordStyle::SpreadThirdAndSeventh))
         *iter += TheScale->GetPitchesPerOctave();
      if ((i == 0 && mChordStyle == ChordStyle::DoubledRoot) ||
          ((i == 0 || i == thirdIndex) && mChordStyle == ChordStyle::DoubledRootAndThird) ||
          (mChordStyle == ChordStyle::Doubled))
         chordPitches.push_back(*iter + TheScale->GetPitchesPerOctave());
      ++iter;
   }

   std::list<int> ret;
   for (int& pitch : chordPitches)
   {
      //snap to scale in scale mode, unless we're intentionally trying to play a different chord
      if (mScaleMode && !GetChordSettings().IsChordButtonPressed())
         pitch = TheScale->MakeDiatonic(pitch);

      if (!ListContains(pitch, ret))
         ret.push_back(pitch);
   }

   ret.sort();

   return ret;
}

//static
int ChordKeyboard::AdjustForVoicing(int pitch, int voicing)
{
   return (pitch + TheScale->GetPitchesPerOctave() - (voicing % TheScale->GetPitchesPerOctave())) % TheScale->GetPitchesPerOctave() + voicing;
}

void ChordKeyboard::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.pitch >= 0 && note.pitch <= 127)
   {
      mNoteMessageQueue.produce(note);
   }
}

const ChordKeyboard::ChordSettings& ChordKeyboard::GetChordSettings() const
{
   if (mChordSettings.mPlayOptions == PlayOptions::ChordOnPress || mChordSettings.mPlayOptions == PlayOptions::ImmediateChord || mChordSettings.mPlayOptions == PlayOptions::ImmediateChordOnly || mInputPitchWrapped == -1)
      return mChordSettings;
   else
      return mLastPlayedChordSettings;
}

void ChordKeyboard::UpdateOutputNotes(double time)
{
   bool forceNoteReplay = false;
   NoteMessage note;
   while (mNoteMessageQueue.consume(note))
   {
      if (!mLatchKey || note.velocity > 0)
      {
         bool noteOn;
         if (mLatchKey)
         {
            if (note.pitch % 12 == mInputPitchWrapped)
               noteOn = false;
            else
               noteOn = true;
         }
         else
         {
            noteOn = note.velocity > 0;
         }

         if (noteOn)
         {
            if (mLatchKey || mQuantizeInterval != kInterval_None)
               forceNoteReplay = true;

            mInputPitchWrapped = note.pitch % 12;
            mLastInputVelocity = note.velocity;
            mLastKeyPressTime = note.time;
            memcpy(&mLastPlayedChordSettings, &mChordSettings, sizeof(mChordSettings));
         }
         else
         {
            if (note.pitch % 12 == mInputPitchWrapped)
            {
               mInputPitchWrapped = -1;
            }
            else if (mInputPitchWrapped != -1)
            {
               if (mQuantizeInterval == kInterval_None)
               {
                  if (note.time < mLastKeyPressTime + 100) // released pitch very shortly after playing new pitch
                  {
                     std::list<int> currentPitches = GetChordPitches(mInputPitchWrapped);
                     std::list<int> previousPitches = GetChordPitches(note.pitch);
                     for (int& pitch : currentPitches)
                     {
                        if (ListContains(pitch, previousPitches))
                        {
                           NoteMessage noteOff(time, pitch, 0);
                           PlayNoteOutput(noteOff);
                           mOutputNotes[pitch] = false; // force it to replay below
                        }
                     }
                  }
               }
            }
         }
      }
   }

   std::array<bool, 128> newOutputNotes{ false };

   std::list<int> chordPitches;
   int bassPitch = -1;
   if (mInputPitchWrapped != -1)
   {
      if (GetChordSettings().ShouldPlayChord())
         chordPitches = GetChordPitches(mInputPitchWrapped);
      if (GetChordSettings().ShouldPlayBass())
         bassPitch = AdjustForVoicing(mInputPitchWrapped, mBassVoicing);
   }

   for (int pitch : chordPitches)
   {
      if (pitch >= 0 && pitch < 127)
         newOutputNotes[pitch] = true;
   }

   int velocity = mLastInputVelocity;
   if (mVelocityOverride != -1)
      velocity = mVelocityOverride;

   for (int i = 0; i < 128; ++i)
   {
      if (newOutputNotes[i] && (!mOutputNotes[i] || forceNoteReplay))
      {
         NoteMessage noteOn(time, i, velocity);
         PlayNoteOutput(noteOn);
         mLastNoteOnTime[i] = time;
      }
      else if (!newOutputNotes[i] && mOutputNotes[i])
      {
         NoteMessage noteOff(time, i, 0);
         PlayNoteOutput(noteOff);
         mLastNoteOffTime[i] = time;
      }
   }

   if (bassPitch != mBassOutputPitch)
   {
      if (mBassOutputPitch != -1)
      {
         NoteMessage noteOff(time, mBassOutputPitch, 0);
         mBassCable->PlayNoteOutput(noteOff);
      }
      if (bassPitch != -1)
      {
         NoteMessage noteOn(time, bassPitch, velocity);
         mBassCable->PlayNoteOutput(noteOn);
      }
   }

   mOutputNotes = newOutputNotes;
   mBassOutputPitch = bassPitch;
}

int ChordKeyboard::GridToPitch(int x, int y) const
{
   int octave = 0;
   if (x >= 7)
      return -1;
   if (y % 2 == 0) //black keys
   {
      const int kBlackKeys[] = { -1, 1, 3, -1, 6, 8, 10 };
      if ((x % 7) == 0 || (x % 7) == 3)
         return -1;
      return kBlackKeys[x % 7] + octave * 12;
   }
   else //white keys
   {
      const int kWhiteKeys[] = { 0, 2, 4, 5, 7, 9, 11 };
      return kWhiteKeys[x % 7] + octave * 12;
   }
}

bool ChordKeyboard::OnAbletonGridControl_InputThread(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue)
{
   int rangeStart = abletonGrid->GetGridStartIndex();
   int rangeEnd = abletonGrid->GetGridStartIndex() + abletonGrid->GetGridNumPads();

   if (controlIndex >= rangeStart && controlIndex < rangeEnd)
   {
      int gridIndex = controlIndex - rangeStart;
      int gridX = gridIndex % abletonGrid->GetGridNumCols();
      int gridY = abletonGrid->GetGridNumRows() - 1 - gridIndex / abletonGrid->GetGridNumCols();

      if (gridY == 0)
      {
         if (mLatchChord)
         {
            if (midiValue > 0)
            {
               if (gridX == 0)
               {
                  mChordSettings.SetDim(!mChordSettings.mDim);
                  OnChordSettingsUpdated(gTime);
               }
               if (gridX == 1)
               {
                  mChordSettings.SetMinor(!mChordSettings.mMinor);
                  OnChordSettingsUpdated(gTime);
               }
               if (gridX == 2)
               {
                  mChordSettings.SetMajor(!mChordSettings.mMajor);
                  OnChordSettingsUpdated(gTime);
               }
               if (gridX == 3)
               {
                  mChordSettings.SetSus(!mChordSettings.mSus);
                  OnChordSettingsUpdated(gTime);
               }
            }
         }
         else
         {
            if (gridX == 0)
            {
               mChordSettings.SetDim(midiValue > 0);
               OnChordSettingsUpdated(gTime);
            }
            if (gridX == 1)
            {
               mChordSettings.SetMinor(midiValue > 0);
               OnChordSettingsUpdated(gTime);
            }
            if (gridX == 2)
            {
               mChordSettings.SetMajor(midiValue > 0);
               OnChordSettingsUpdated(gTime);
            }
            if (gridX == 3)
            {
               mChordSettings.SetSus(midiValue > 0);
               OnChordSettingsUpdated(gTime);
            }
         }

         if (gridX == 4 && midiValue > 0)
         {
            mLatchChord = !mLatchChord;
            abletonGrid->DisplayScreenMessage(std::string("latch chord: ") + (mLatchChord ? "true" : "false"));
         }

         if (gridX == 7 && midiValue > 0)
            mScaleMode = !mScaleMode;
      }
      if (gridY == 1)
      {
         if (mLatchChord)
         {
            if (midiValue > 0)
            {
               if (gridX == 0)
               {
                  mChordSettings.mSix = !mChordSettings.mSix;
                  OnChordSettingsUpdated(gTime);
               }
               if (gridX == 1)
               {
                  mChordSettings.mMin7 = !mChordSettings.mMin7;
                  OnChordSettingsUpdated(gTime);
               }
               if (gridX == 2)
               {
                  mChordSettings.mMaj7 = !mChordSettings.mMaj7;
                  OnChordSettingsUpdated(gTime);
               }
               if (gridX == 3)
               {
                  mChordSettings.mNine = !mChordSettings.mNine;
                  OnChordSettingsUpdated(gTime);
               }
            }
         }
         else
         {
            if (gridX == 0)
            {
               mChordSettings.mSix = midiValue > 0;
               OnChordSettingsUpdated(gTime);
            }
            if (gridX == 1)
            {
               mChordSettings.mMin7 = midiValue > 0;
               OnChordSettingsUpdated(gTime);
            }
            if (gridX == 2)
            {
               mChordSettings.mMaj7 = midiValue > 0;
               OnChordSettingsUpdated(gTime);
            }
            if (gridX == 3)
            {
               mChordSettings.mNine = midiValue > 0;
               OnChordSettingsUpdated(gTime);
            }
         }

         if (gridX == 4 && midiValue > 0)
         {
            mLatchKey = !mLatchKey;
            abletonGrid->DisplayScreenMessage(std::string("latch keyboard: ") + (mLatchKey ? "true" : "false"));
         }
      }
      if (gridY >= 2)
      {
         int pitch = GridToPitch(gridX, gridY);

         if (pitch != -1)
            PlayNote(NoteMessage(gTime, pitch, midiValue));
      }

      return true;
   }

   if (controlIndex == AbletonDevice::kClickyEncoderTurn)
   {
      int direction = midiValue <= 64 ? 1 : -1;
      mVoicing = ofClamp(mVoicing + direction, mVoicingSlider->GetMin(), mVoicingSlider->GetMax());
      //UpdateOutputNotes(gTime);
   }

   return false;
}

bool ChordKeyboard::OnAbletonGridControl(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue)
{
   return false;
}

void ChordKeyboard::UpdateAbletonGridLeds(IAbletonGridDevice* abletonGrid)
{
   int rangeStart = abletonGrid->GetGridStartIndex();
   int rangeEnd = abletonGrid->GetGridStartIndex() + abletonGrid->GetGridNumPads();

   for (int controlIndex = rangeStart; controlIndex <= rangeEnd; ++controlIndex)
   {
      int gridIndex = controlIndex - rangeStart;
      int gridX = gridIndex % abletonGrid->GetGridNumCols();
      int gridY = abletonGrid->GetGridNumRows() - 1 - gridIndex / abletonGrid->GetGridNumCols();

      int color = AbletonDevice::kColorOff;
      int flashColor = -1;
      if (gridY == 0)
      {
         if (gridX == 0)
            color = mChordSettings.mDim ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;
         if (gridX == 1)
            color = mChordSettings.mMinor ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;
         if (gridX == 2)
            color = mChordSettings.mMajor ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;
         if (gridX == 3)
            color = mChordSettings.mSus ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;

         if (gridX == 4)
            color = mLatchChord ? AbletonDevice::kColorOrange : AbletonDevice::kColorSoftPeach;

         if (gridX == 7)
            color = mScaleMode ? AbletonDevice::kColorBlue : AbletonDevice::kColorPaleBlue;
      }
      if (gridY == 1)
      {
         if (gridX == 0)
            color = mChordSettings.mSix ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;
         if (gridX == 1)
            color = mChordSettings.mMin7 ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;
         if (gridX == 2)
            color = mChordSettings.mMaj7 ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;
         if (gridX == 3)
            color = mChordSettings.mNine ? AbletonDevice::kColorBrightPink : AbletonDevice::kColorPaleMagenta;

         if (gridX == 4)
            color = mLatchKey ? AbletonDevice::kColorOrange : AbletonDevice::kColorSoftPeach;
      }
      if (gridY >= 2)
      {
         int pitch = GridToPitch(gridX, gridY);

         if (pitch >= 0 && pitch < 127)
         {
            if (mInputPitchWrapped == pitch)
               color = AbletonDevice::kColorGreen;
            else if (mScaleMode && !TheScale->IsInScale(pitch))
               color = AbletonDevice::kColorDimRed;
            else if (gridY == 2)
               color = AbletonDevice::kColorDarkGreen;
            else
               color = AbletonDevice::kColorMintGreen;

            if (mScaleMode && TheScale->IsRoot(pitch))
               flashColor = AbletonDevice::kColorWhite;
         }
      }

      abletonGrid->SetLed(controlIndex, color, flashColor);
   }
}

std::string ChordKeyboard::GetChordDisplayString() const
{
   std::string displayChordName;
   std::string chord;

   if (GetChordSettings().mDim)
      chord += "dim";
   if (GetChordSettings().mMinor)
      chord += "m";
   if (GetChordSettings().mMajor)
      chord += "";
   if (GetChordSettings().mSus)
      chord += "sus";

   if (mScaleMode && mInputPitchWrapped != -1 && !GetChordSettings().IsChordButtonPressed())
   {
      if (!TheScale->IsInScale(mInputPitchWrapped + 7)) //diminished chord
         chord += "dim";
      else if (TheScale->IsInScale(mInputPitchWrapped + 3)) //minor chord
         chord += "m";
      else
         chord += "";
   }

   if (GetChordSettings().mSix)
      chord += "6";
   if (GetChordSettings().mMin7)
      chord += "7";
   if (GetChordSettings().mMaj7)
      chord += "M7";
   if (GetChordSettings().mNine)
      chord += "9";

   if (displayChordName == "" && GetChordSettings().mMajor)
      displayChordName = "Maj"; //major

   if (mInputPitchWrapped != -1)
      return NoteName(mInputPitchWrapped) + chord;
   else
      return chord;
}

bool ChordKeyboard::UpdateAbletonMoveScreen(IAbletonGridDevice* abletonGrid, AbletonMoveLCD* lcd, LCDDrawPass drawPass)
{
   for (int i = AbletonDevice::kMainEncoderTouchSection; i < AbletonDevice::kMainEncoderTouchSection + AbletonDevice::kNumMainEncoders; ++i)
   {
      if (abletonGrid->GetButtonState(i))
         return false;
   }

   std::string chordPitchNames;
   if (mInputPitchWrapped != -1)
      chordPitchNames += GetRomanNumeralForDegree(TheScale->GetToneFromPitch(mInputPitchWrapped)) + ": ";
   for (int i = 0; i < 128; ++i)
   {
      if (mOutputNotes[i])
         chordPitchNames += NoteName(i, false, true) + " ";
   }

   lcd->DrawLCDText(chordPitchNames.c_str(), 10, 15, 0, 12);

   if (abletonGrid->GetButtonState(AbletonDevice::kClickyEncoderTouch))
   {
      lcd->DrawLCDText("voicing:", 10, 25, 0, 12);
      lcd->DrawLCDText(ofToString(mVoicing).c_str(), 10, 60, 0, 50);
      return true;
   }

   bool chordButtonHeld = false;
   int rangeStart = abletonGrid->GetGridStartIndex();
   int rangeEnd = abletonGrid->GetGridStartIndex() + abletonGrid->GetGridNumPads();
   for (int controlIndex = rangeStart; controlIndex <= rangeEnd; ++controlIndex)
   {
      int gridIndex = controlIndex - rangeStart;
      int gridX = gridIndex % abletonGrid->GetGridNumCols();
      int gridY = abletonGrid->GetGridNumRows() - 1 - gridIndex / abletonGrid->GetGridNumCols();
      if (gridX < 4 && gridY < 2 && abletonGrid->GetButtonState(controlIndex))
         chordButtonHeld = true;
   }

   if (mInputPitchWrapped != -1 || chordButtonHeld)
   {
      std::string displayString = GetChordDisplayString();

      if (mInputPitchWrapped != -1)
         lcd->DrawLCDText(displayString.c_str(), 10, 60, 0, 50);
      else
         lcd->DrawLCDText(displayString.c_str(), 10, 30, 0, 12);

      /*std::vector<int> chordPitches;
      for (int i=0; i<128; ++i)
      {
         if (mOutputNotes[i])
            chordPitches.push_back(i);
      }
      displayString = TheScale->GetChordDatabase().GetChordName(chordPitches, mInputPitchWrapped);*/

      return true;
   }

   return false;
}

void ChordKeyboard::GetPush2OverrideControls(std::vector<IUIControl*>& controls) const
{
   controls.push_back(mBassVoicingSlider);
   controls.push_back(mVelocityOverrideSlider);
   controls.push_back(mPlayOptionsSelector);
   controls.push_back(mChordStyleSelector);
   controls.push_back(mQuantizeIntervalSelector);
}

void ChordKeyboard::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ChordKeyboard::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void ChordKeyboard::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void ChordKeyboard::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());
}
