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
//  ChaosEngine.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/18/13.
//
//

#include "ChaosEngine.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Transport.h"
#include "Profiler.h"

ChaosEngine* TheChaosEngine = nullptr;

ChaosEngine::ChaosEngine()
{
   assert(TheChaosEngine == nullptr);
   TheChaosEngine = this;

   mChordProgression.push_back(ProgressionChord(0));
}

void ChaosEngine::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mChaosButton = new ClickButton(this, "CHAOS", 10, 110);
   mTotalChaosCheckbox = new Checkbox(this, "total chaos", 10, 130, &mTotalChaos);
   mSectionDropdown = new RadioButton(this, "section", 10, 450, &mSectionIdx, kRadioHorizontal);
   mSongDropdown = new DropdownList(this, "song", 200, 4, &mSongIdx);
   mReadSongsButton = new ClickButton(this, "read", 350, 4);
   mPlayChordCheckbox = new Checkbox(this, "play chord", 400, 4, &mPlayChord);
   mProgressCheckbox = new Checkbox(this, "progress", 500, 4, &mProgress);
   mDegreeSlider = new IntSlider(this, "degree", 400, 24, 80, 15, &mDegree, 0, 6);
   mRootNoteList = new DropdownList(this, "rootnote", 490, 24, &mInputChord.mRootPitch);
   mChordTypeList = new DropdownList(this, "chord type", 530, 24, (int*)(&mInputChord.mType));
   mAddChordButton = new ClickButton(this, "add", 550, 60);
   mRemoveChordButton = new ClickButton(this, "remove", 550, 80);
   mSetProgressionButton = new ClickButton(this, "set", 550, 110);
   mChordProgressionSlider = new IntSlider(this, "progression", 400, 450, 100, 15, &mChordProgressionIdx, 0, 0);
   mRandomProgressionButton = new ClickButton(this, "r progression", 510, 450);
   mInversionSlider = new IntSlider(this, "inversion", 500, 40, 80, 15, &mInputChord.mInversion, 0, 2);
   mHideBeatCheckbox = new Checkbox(this, "hide beat", 112, 4, &mHideBeat);

   mRootNoteList->AddLabel("A", 9);
   mRootNoteList->AddLabel("A#", 10);
   mRootNoteList->AddLabel("B", 11);
   mRootNoteList->AddLabel("C", 0);
   mRootNoteList->AddLabel("C#", 1);
   mRootNoteList->AddLabel("D", 2);
   mRootNoteList->AddLabel("D#", 3);
   mRootNoteList->AddLabel("E", 4);
   mRootNoteList->AddLabel("F", 5);
   mRootNoteList->AddLabel("F#", 6);
   mRootNoteList->AddLabel("G", 7);
   mRootNoteList->AddLabel("G#", 8);

   mChordTypeList->AddLabel("maj ", kChord_Maj);
   mChordTypeList->AddLabel("min ", kChord_Min);
   mChordTypeList->AddLabel("aug ", kChord_Aug);
   mChordTypeList->AddLabel("dim ", kChord_Dim);
}

ChaosEngine::~ChaosEngine()
{
   assert(TheChaosEngine == this || TheChaosEngine == nullptr);
   TheChaosEngine = nullptr;
}

void ChaosEngine::Init()
{
   IDrawableModule::Init();

   ReadSongs();
}

void ChaosEngine::Poll()
{
   double chaosTimeLeft = mChaosArrivalTime - gTime;
   if (chaosTimeLeft > 0)
   {
      if (chaosTimeLeft > ofRandom(-1000, 3000))
      {
         if (mTotalChaos)
            TheTransport->SetTimeSignature(gRandom() % 8 + 2, (int)powf(2, gRandom() % 3 + 2));

         TheScale->SetRoot(gRandom() % TheScale->GetPitchesPerOctave());
         TheScale->SetRandomSeptatonicScale();
         float bias = ofRandom(0, 1);
         bias *= bias;
         TheTransport->SetTempo(ofMap(bias, 0.0f, 1.0f, 55.0f, 170.0f));
      }
   }
}

void ChaosEngine::AudioUpdate()
{
   PROFILER(ChaosEngine);

   int measure = TheTransport->GetMeasure(gTime);
   int beat = int(TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop());

   if (beat != mLastAudioUpdateBeat && mBeatsLeftToChordChange != -1)
   {
      --mBeatsLeftToChordChange;
   }

   if (mProgress && (measure != mLastAudioUpdateMeasure || mBeatsLeftToChordChange == 0))
   {
      if (mRestarting)
      {
         mRestarting = false;
      }
      else
      {
         mChordProgressionIdx = (mChordProgressionIdx + 1) % mChordProgression.size();
         UpdateProgression(beat);
      }
   }

   mLastAudioUpdateMeasure = measure;
   mLastAudioUpdateBeat = beat;
}

void ChaosEngine::UpdateProgression(int beat)
{
   mProgressionMutex.lock();

   if (mChordProgressionIdx < 0 || mChordProgressionIdx >= mChordProgression.size())
   {
      mProgressionMutex.unlock();
      return;
   }

   if (mSongIdx != -1 || mChordProgression.size() > 1)
      TheScale->SetScaleDegree(mChordProgression[mChordProgressionIdx].mDegree);
   TheScale->SetAccidentals(mChordProgression[mChordProgressionIdx].mAccidentals);
   if (mChordProgression[mChordProgressionIdx].mBeatLength != -1)
      mBeatsLeftToChordChange = mChordProgression[mChordProgressionIdx].mBeatLength;
   else
      mBeatsLeftToChordChange = TheTransport->GetTimeSigTop() - beat;
   mProgressionMutex.unlock();

   mNoteOutput.Flush(NextBufferTime(false));
   if (mPlayChord)
   {
      std::vector<int> pitches = GetCurrentChordPitches();
      for (int i = 0; i < pitches.size(); ++i)
      {
         PlayNoteOutput(NoteMessage(gTime, pitches[i], 127));
      }
   }
}

std::vector<int> ChaosEngine::GetCurrentChordPitches()
{
   ProgressionChord chord(0);
   if (mChordProgressionIdx != -1)
      chord = mChordProgression[mChordProgressionIdx];

   int degree = TheScale->GetScaleDegree();

   std::vector<int> tones;
   tones.push_back(0 + degree);
   tones.push_back(2 + degree);
   tones.push_back(4 + degree);

   std::vector<int> pitches;
   for (int i = 0; i < tones.size(); ++i)
   {
      int tone = tones[i];
      if (i >= tones.size() - chord.mInversion)
         tone -= 7;
      pitches.push_back(TheScale->GetPitchFromTone(tone) + 48);
   }

   return pitches;
}

void ChaosEngine::DrawModule()
{
   ofSetColor(255, 255, 255);

   ofPushStyle();

   float fBeat = TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop() + 1;
   int beat = int(fBeat);
   if (!mHideBeat)
   {
      float w, h;
      GetDimensions(w, h);
      ofFill();
      float beatLeft = 1 - (fBeat - beat);
      ofSetColor(255, 255, 255, 100 * beatLeft);
      float beatWidth = w / TheTransport->GetTimeSigTop();
      ofRect((beat - 1) * beatWidth, 0, beatWidth, h);
      ofPopStyle();
   }

   if (Minimized() || IsVisible() == false)
      return;

   mChaosButton->Draw();
   mTotalChaosCheckbox->Draw();
   mSectionDropdown->Draw();
   mSongDropdown->Draw();
   mReadSongsButton->Draw();
   mPlayChordCheckbox->Draw();
   mProgressCheckbox->Draw();
   mDegreeSlider->Draw();
   mRootNoteList->Draw();
   mChordTypeList->Draw();
   mAddChordButton->Draw();
   mRemoveChordButton->Draw();
   mSetProgressionButton->Draw();
   mChordProgressionSlider->Draw();
   mRandomProgressionButton->Draw();
   mInversionSlider->Draw();
   mHideBeatCheckbox->Draw();

   DrawTextNormal(mInputChord.Name(true, true), 400, 51);

   for (int i = 0; i < mInputChords.size(); ++i)
   {
      int degree;
      std::vector<Accidental> accidentals;
      TheScale->GetChordDegreeAndAccidentals(mInputChords[i], degree, accidentals);
      std::string accidentalList;
      for (int j = 0; j < accidentals.size(); ++j)
         accidentalList += ofToString(accidentals[j].mPitch) + (accidentals[j].mDirection == 1 ? "#" : "b") + " ";
      DrawTextNormal(mInputChords[i].Name(true, true), 400, 75 + i * 15);
   }

   if (!mHideBeat)
      gFont.DrawString(ofToString(beat), 42, 10, 50);
   gFont.DrawString(ofToString(TheTransport->GetTimeSigTop()) + "/" + ofToString(TheTransport->GetTimeSigBottom()), 40, 90, 50);
   gFont.DrawString(ofToString(TheTransport->GetTempo(), 0) + "bpm", 420, 230, 50);

   gFont.DrawString(NoteName(TheScale->ScaleRoot()) + " " + TheScale->GetType(), 40, 10, 100);

   DrawKeyboard(10, 150);
   DrawGuitar(10, 285);

   ofPushStyle();
   ofFill();
   for (int i = 0; i < mChordProgression.size(); ++i)
   {
      if (i == mChordProgressionIdx)
         ofSetColor(255, 255, 255);
      else
         ofSetColor(150, 150, 150);

      int x = 10 + 290 * (i / 6);
      int y = 500 + 35 * (i % 6);

      ScalePitches scale;
      scale.SetRoot(TheScale->ScaleRoot());
      scale.SetScaleType(TheScale->GetType());
      scale.SetAccidentals(mChordProgression[i].mAccidentals);

      Chord displayChord;
      displayChord.SetFromDegreeAndScale(mChordProgression[i].mDegree, scale);
      displayChord.mInversion = mChordProgression[i].mInversion;
      gFont.DrawString(displayChord.Name(true, false, &scale), 46, x, y);

      std::string accidentalList;
      for (int j = 0; j < mChordProgression[i].mAccidentals.size(); ++j)
         accidentalList += ofToString(mChordProgression[i].mAccidentals[j].mPitch) + (mChordProgression[i].mAccidentals[j].mDirection == 1 ? "#" : "b") + "\n";
      DrawTextNormal(accidentalList, x + 180, y - 18);

      int numBeats = mChordProgression[i].mBeatLength;
      if (numBeats == -1)
         numBeats = TheTransport->GetTimeSigTop();
      for (int j = 0; j < numBeats; ++j)
      {
         if (i < mChordProgressionIdx ||
             (i == mChordProgressionIdx && j <= numBeats - mBeatsLeftToChordChange))
            ofSetColor(255, 255, 255);
         else
            ofSetColor(150, 150, 150);
         ofRect(x + 210 + j * 15, y - 25, 13, 25);
      }
   }
   ofPopStyle();
}

void ChaosEngine::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   if (y > 470)
   {
      y -= 470;
      x -= 10;

      int xSlot = x / 290;
      int ySlot = y / 35;

      int idx = ySlot + xSlot * 6;

      if (idx < mChordProgression.size())
      {
         if (IsKeyHeld('x') && mChordProgression.size() > 1)
         {
            mProgressionMutex.lock();
            mChordProgression.erase(mChordProgression.begin() + idx);
            mChordProgressionSlider->SetExtents(0, (int)mChordProgression.size() - 1);
            mProgressionMutex.unlock();
         }
         else
         {
            mChordProgressionIdx = idx;
            UpdateProgression(0);
         }
      }
   }
}

ofRectangle ChaosEngine::GetKeyboardKeyRect(int pitch, bool& isBlackKey)
{
   const float kbWidth = 200;
   const float kbHeight = 100;

   int offset = pitch / TheScale->GetPitchesPerOctave() * (kbWidth - kbWidth / 8);
   pitch %= 12;

   if ((pitch <= 4 && pitch % 2 == 0) || (pitch >= 5 && pitch % 2 == 1)) //white key
   {
      int whiteKey = (pitch + 1) / 2;
      isBlackKey = false;
      return ofRectangle(offset + whiteKey * kbWidth / 8, 0, kbWidth / 8, kbHeight);
   }
   else //black key
   {
      int blackKey = pitch / 2;
      isBlackKey = true;
      return ofRectangle(offset + blackKey * kbWidth / 8 + kbWidth / 16 + kbWidth / 8 * .1f, 0, kbWidth / 8 * .8f, kbHeight / 2);
   }
}

void ChaosEngine::DrawKeyboard(float x, float y)
{
   ofPushStyle();
   ofPushMatrix();
   ofTranslate(x, y);

   const int kNumKeys = 41;

   //white keys
   for (int pass = 0; pass < 2; ++pass)
   {
      for (int i = 0; i < kNumKeys; ++i)
      {
         bool isBlackKey;
         ofRectangle key = GetKeyboardKeyRect(i, isBlackKey);

         if ((pass == 0 && !isBlackKey) || (pass == 1 && isBlackKey))
         {
            SetPitchColor(i + 12);
            ofFill();
            ofRect(key);
            ofSetColor(0, 0, 0);
            ofNoFill();
            ofRect(key);
         }
      }
   }

   ofPushStyle();
   ofFill();
   ofSetColor(255, 255, 255);
   ofSetLineWidth(2);
   std::vector<int> chord = GetCurrentChordPitches();
   sort(chord.begin(), chord.end());
   ofVec2f lastNoteConnector;
   for (int j = 0; j < chord.size(); ++j)
   {
      bool isBlackKey;
      int pitch = chord[j] - 36;
      if (pitch < kNumKeys)
      {
         ofRectangle key = GetKeyboardKeyRect(pitch, isBlackKey);
         key.height /= 3;
         key.y += key.height * 2;

         ofRect(key);

         if (IsChordRoot(pitch))
         {
            ofPushStyle();
            ofSetColor(0, 0, 0);
            ofCircle(key.x + key.width / 2, key.y + key.height / 2, 5);
            ofPopStyle();
         }

         if (j > 0)
            ofLine(lastNoteConnector.x, lastNoteConnector.y, key.x, key.y + key.height / 2);

         lastNoteConnector.set(key.x + key.width, key.y + key.height / 2);
      }
   }
   ofPopStyle();

   ofPopMatrix();
   ofPopStyle();
}

void ChaosEngine::DrawGuitar(float x, float y)
{
   ofPushStyle();
   ofPushMatrix();
   ofTranslate(x, y);
   const float gtWidth = 570;
   const float gtHeight = 150;

   const int numStrings = 6;
   const int numFrets = 13;
   const float fretWidth = gtWidth / numFrets;

   ofSetCircleResolution(10);
   ofFill();

   ofSetColor(0, 0, 0);
   ofRect(fretWidth, 0, gtWidth - fretWidth, gtHeight);

   ofSetLineWidth(3);

   for (int fret = 1; fret < numFrets; ++fret)
   {
      float fretX = fret * fretWidth;

      ofSetColor(200, 200, 200);
      ofLine(fretX, 0, fretX, gtHeight);

      if (fret == 3 || fret == 5 || fret == 7 || fret == 9 || fret == 15 || fret == 17 || fret == 19 || fret == 21)
      {
         ofSetColor(255, 255, 255);
         ofCircle(fretX + fretWidth / 2, gtHeight / 2, 5);
      }

      if (fret == 12)
      {
         ofSetColor(255, 255, 255);
         ofCircle(fretX + fretWidth / 2, gtHeight * .3f, 5);
         ofCircle(fretX + fretWidth / 2, gtHeight * .7f, 5);
      }
   }

   ofSetLineWidth(1);

   for (int string = 0; string < numStrings; ++string)
   {
      float stringY = gtHeight - string * gtHeight / (numStrings - 1);
      ofSetColor(255, 255, 255);
      ofLine(0, stringY, gtWidth, stringY);
   }

   ofSetLineWidth(2);

   for (int string = 0; string < numStrings; ++string)
   {
      float stringY = gtHeight - string * gtHeight / (numStrings - 1);

      for (int fret = 0; fret < numFrets; ++fret)
      {
         float fretX = fret * fretWidth;

         int pitch = 16 + fret + string * 5; //16 to get to E-1
         if (string >= 5)
            pitch -= 1;

         if (TheScale->IsInScale(pitch))
         {
            SetPitchColor(pitch);
            if (fret == 0)
               ofNoFill();
            else
               ofFill();
            ofCircle(fretX + fretWidth / 2, stringY, 9);

            if (IsChordRoot(pitch))
            {
               ofNoFill();
               ofSetColor(255, 255, 255);
               ofCircle(fretX + fretWidth / 2, stringY, 10);
            }
         }
      }
   }

   ofPopMatrix();
   ofPopStyle();
}

void ChaosEngine::SetPitchColor(int pitch)
{
   if (TheScale->IsRoot(pitch))
   {
      ofSetColor(0, 255, 0);
   }
   else if (TheScale->IsInPentatonic(pitch))
   {
      ofSetColor(255, 128, 0);
   }
   else if (TheScale->IsInScale(pitch))
   {
      ofSetColor(180, 80, 0);
   }
   else
   {
      ofSetColor(50, 50, 50);
   }
}

bool ChaosEngine::IsChordRoot(int pitch)
{
   if (mChordProgression.size() <= 1) //no chords
      return false;

   return TheScale->IsInScale(pitch) &&
          (TheScale->GetToneFromPitch(pitch) % 7 == (TheScale->GetScaleDegree() + 7) % 7);
}

void ChaosEngine::ReadSongs()
{
   mSongDropdown->Clear();
   mSongDropdown->AddLabel("none", -1);

   ofxJSONElement root;
   bool loaded = root.open(ofToDataPath("songs.json"));
   assert(loaded);

   const ofxJSONElement& songs = root["songs"];
   mSongs.resize(songs.size());
   //TODO(Ryan) this is broken. but is it worth reviving?
   /*for (int i=0; i<songs.size(); ++i)
   {
      const ofxJSONElement& song = songs[i];
      mSongs[i].mName = song["name"].asString();
      mSongs[i].mTempo = song["tempo"].asDouble();
      mSongs[i].mTimeSigTop = song["timesig"][0u].asInt();
      mSongs[i].mTimeSigBottom = song["timesig"][1u].asInt();
      
      std::string scaleRootName = song["scaleroot"].asString();
      int j;
      for (j=0; j<12; ++j)
      {
         if (scaleRootName == NoteName(j))
         {
            mSongs[i].mScaleRoot = j;
            break;
         }
      }
      assert(j != 12);
      
      mSongs[i].mScaleType = song["scaletype"].asString();
      
      ScalePitches scale;
      scale.SetRoot(mSongs[i].mScaleRoot);
      scale.SetScaleType(mSongs[i].mScaleType);
      
      const ofxJSONElement& sections = song["sections"];
      mSongs[i].mSections.resize(sections.size());
      for (j=0; j<sections.size(); ++j)
      {
         const ofxJSONElement& section = sections[j];
         mSongs[i].mSections[j].mName = section["name"].asString();
         mSongs[i].mSections[j].mChords.clear();
         
         for (int k=0; k<section["chords"].size(); ++k)
         {
            const ofxJSONElement& chordInfo = section["chords"][k];
         
            mSongs[i].mSections[j].mChords.push_back(ProgressionChord(chordInfo,scale));
         }
      }
   }*/

   for (int i = 0; i < mSongs.size(); ++i)
      mSongDropdown->AddLabel(mSongs[i].mName.c_str(), i);
}

void ChaosEngine::GenerateRandomProgression()
{
   mProgressionMutex.lock();

   std::vector<Accidental> none;
   TheScale->SetAccidentals(none);

   mChordProgression.clear();
   int length = (gRandom() % 3) + 3;
   mChordProgression.push_back(ProgressionChord(0));
   int beatsPerBar = TheTransport->GetTimeSigTop();
   int beatsLeft = beatsPerBar;
   for (int i = 1; i < length;)
   {
      int degree = (gRandom() % 6) + 1;
      Chord chord;
      int rootPitch = TheScale->GetPitchFromTone(degree);
      if (ofRandom(1) < .5f && (degree == 3 || degree == 4)) //chance we do a non-diatonic
         chord = Chord(rootPitch, (gRandom() % 2) ? kChord_Maj : kChord_Min);
      else
         chord.SetFromDegreeAndScale(degree, TheScale->GetScalePitches());

      std::vector<Accidental> accidentals;
      TheScale->GetChordDegreeAndAccidentals(chord, degree, accidentals);

      int beats = beatsLeft;
      if (ofRandom(1) < .2f) //small chance we do less than a bar
         beats = (gRandom() % beatsLeft) + 1;

      ProgressionChord progressionChord(degree, accidentals, beats);

      const ProgressionChord& lastChord = mChordProgression[mChordProgression.size() - 1];
      if (progressionChord.SameChord(lastChord)) //two in a row
         continue;

      progressionChord.mInversion = gRandom() % 3;

      mChordProgression.push_back(progressionChord);
      beatsLeft -= beats;
      if (beatsLeft == 0)
      {
         beatsLeft = beatsPerBar;
         ++i;
      }
   }

   mChordProgressionSlider->SetExtents(0, (int)mChordProgression.size() - 1);

   mProgressionMutex.unlock();
}

ChaosEngine::ProgressionChord::ProgressionChord(const ofxJSONElement& chordInfo, ScalePitches scale)
{
   if (chordInfo.isArray())
   {
      if (chordInfo[0u].isInt())
      {
         mDegree = chordInfo[0u].asInt();
         mBeatLength = chordInfo[1u].asInt();
      }
      else
      {
         scale.GetChordDegreeAndAccidentals(chordInfo[0u].asString(), mDegree, mAccidentals);
         mBeatLength = chordInfo[1u].asInt();
         if (chordInfo[2u].isNull())
            mInversion = 0;
         else
            mInversion = chordInfo[2u].asInt();
      }
   }
   else
   {
      if (chordInfo.isInt())
         mDegree = chordInfo.asInt();
      else
         scale.GetChordDegreeAndAccidentals(chordInfo.asString(), mDegree, mAccidentals);
      mBeatLength = -1;
      mInversion = 0;
   }
}

void ChaosEngine::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mSongDropdown)
   {
      mProgressionMutex.lock();

      if (mSongIdx == -1)
      {
         mChordProgression.clear();
         mChordProgression.push_back(0);
         mSectionDropdown->Clear();
      }
      else
      {
         const Song& song = mSongs[mSongIdx];

         TheTransport->SetTimeSignature(song.mTimeSigTop, song.mTimeSigBottom);
         TheTransport->SetTempo(song.mTempo);
         TheScale->SetRoot(song.mScaleRoot);
         TheScale->SetScaleType(song.mScaleType);

         mChordProgression = song.mSections[0].mChords;

         mSectionDropdown->Clear();
         for (int i = 0; i < song.mSections.size(); ++i)
         {
            mSectionDropdown->AddLabel(song.mSections[i].mName.c_str(), i);
         }

         mChordProgressionIdx = -1;
         mBeatsLeftToChordChange = -1;
         mSectionIdx = 0;
      }

      mChordProgressionSlider->SetExtents(0, (int)mChordProgression.size() - 1);
      mProgressionMutex.unlock();
   }
   if (list == mRootNoteList || list == mChordTypeList)
   {
      mDegree = TheScale->GetToneFromPitch(mInputChord.mRootPitch);
   }
}

void ChaosEngine::ButtonClicked(ClickButton* button, double time)
{
   if (button == mChaosButton)
   {
      mChaosArrivalTime = time + ofRandom(2000, 3000);
   }
   if (button == mReadSongsButton)
   {
      ReadSongs();
   }
   if (button == mAddChordButton)
   {
      mInputChords.push_back(mInputChord);
   }
   if (button == mRemoveChordButton)
   {
      if (mInputChords.size())
         mInputChords.pop_back();
   }
   if (button == mSetProgressionButton)
   {
      mProgressionMutex.lock();
      mChordProgression.clear();
      for (int i = 0; i < mInputChords.size(); ++i)
      {
         int degree;
         std::vector<Accidental> accidentals;
         TheScale->GetChordDegreeAndAccidentals(mInputChords[i], degree, accidentals);
         ProgressionChord chord = ProgressionChord(degree, accidentals);
         chord.mInversion = mInputChords[i].mInversion;
         mChordProgression.push_back(chord);
      }
      mProgressionMutex.unlock();
      mChordProgressionSlider->SetExtents(0, (int)mChordProgression.size() - 1);
   }
   if (button == mRandomProgressionButton)
   {
      GenerateRandomProgression();
   }
}

void ChaosEngine::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void ChaosEngine::RadioButtonUpdated(RadioButton* list, int oldVal, double time)
{
   if (list == mSectionDropdown)
   {
      if (mSongIdx == -1 || mSectionIdx >= mSongs[mSongIdx].mSections.size())
         return;

      mProgressionMutex.lock();

      mChordProgression = mSongs[mSongIdx].mSections[mSectionIdx].mChords;
      mChordProgressionIdx = -1;

      mProgressionMutex.unlock();

      mChordProgressionSlider->SetExtents(0, (int)mChordProgression.size() - 1);
   }
}

void ChaosEngine::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mDegreeSlider)
   {
      mInputChord.SetFromDegreeAndScale(mDegree, TheScale->GetScalePitches());
   }
   if (slider == mChordProgressionSlider)
   {
      mChordProgressionIdx = ofClamp(mChordProgressionIdx, 0, mChordProgression.size() - 1);
      if (mProgress)
         mChordProgressionIdx -= 1; //make us progress to this one on the next downbeat
      else
         UpdateProgression(0); //or go immediately there
   }
}

void ChaosEngine::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ChaosEngine::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
