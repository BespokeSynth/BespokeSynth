#include "Maze.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "Scale.h"
#include "SynthGlobals.h"

Maze::Maze()
{
}

Maze::~Maze()
{
   TheTransport->RemoveListener(this);
}

void Maze::Init()
{
   IDrawableModule::Init();
   mTransportListenerInfo = TheTransport->AddListener(this, kInterval_64n, OffsetInfo(0, true), true);
}

void Maze::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float x = 10;
   float y = 10;

   mScaledDegreeCheckbox = new Checkbox(this, "scaled degree", x, y, &mScaledDegree);
   y += 20;

   mNumChordsDropdown = new DropdownList(this, "no of chords", x, y, &mNumChordsIndex);
   for (int i = 1; i <= 16; ++i)
      mNumChordsDropdown->AddLabel(ofToString(i), i - 1);
   mNumChordsIndex = 3; // default 4 chords
   y += 20;

   mRandomiseButton = new ClickButton(this, "randomise", x, y);
   y += 20;

   mGrooveSlider = new FloatSlider(this, "groove", x, y, 125, 15, &mGroove, 0.0f, 1.0f);
   y += 20;

   mHumaniseVelocitySlider = new FloatSlider(this, "humanise vel", x, y, 125, 15, &mHumaniseVelocity, 0.0f, 1.0f);
   y += 20;

   mUpperHarmonicsSlider = new FloatSlider(this, "upper harmonics", x, y, 125, 15, &mUpperHarmonics, 0.0f, 1.0f);
   y += 20;

   mStrumSlider = new FloatSlider(this, "strum", x, y, 125, 15, &mStrum, 0.0f, 100.0f);
   y += 20;

   mHumaniseTimingSlider = new FloatSlider(this, "humanise timing", x, y, 125, 15, &mHumaniseTiming, 0.0f, 50.0f);
   y += 20;

   GenerateChords();
}

void Maze::DrawModule()
{
   mScaledDegreeCheckbox->Draw();
   mNumChordsDropdown->Draw();
   mRandomiseButton->Draw();
   mGrooveSlider->Draw();
   mHumaniseVelocitySlider->Draw();
   mUpperHarmonicsSlider->Draw();
   mStrumSlider->Draw();
   mHumaniseTimingSlider->Draw();

   float gridStartX = 140;
   float gridStartY = 10;
   float sqSize = 28.0f;

   ofPushStyle();
   for (int row = 0; row < 8; ++row)
   {
      for (int col = 0; col < 8; ++col)
      {
         int pitch = 48 + row * 8 + col;
         bool isLocked = mLockedNotes.count(pitch) > 0;
         bool isActive = false;

         if (mCurrentChordIndex >= 0 && mCurrentChordIndex < mChords.size())
         {
            const auto& currentChord = mChords[mCurrentChordIndex];
            isActive = std::find(currentChord.pitches.begin(), currentChord.pitches.end(), pitch) != currentChord.pitches.end();
         }

         bool inScale = TheScale->IsInScale(pitch);

         if (isActive)
            ofSetColor(255, 200, 50);
         else if (isLocked)
            ofSetColor(200, 100, 150); // Locked note color
         else
            ofSetColor(50, 50, 50);

         ofFill();
         ofRect(gridStartX + col * sqSize, gridStartY + (7 - row) * sqSize, sqSize - 2, sqSize - 2);

         if (isLocked)
            ofSetColor(255, 150, 200);
         else if (inScale)
            ofSetColor(120, 120, 255); // Highlight in-scale notes with blue outline
         else
            ofSetColor(100);
         ofNoFill();
         ofRect(gridStartX + col * sqSize, gridStartY + (7 - row) * sqSize, sqSize - 2, sqSize - 2);
      }
   }
   ofPopStyle();
}

void Maze::GetModuleDimensions(float& width, float& height)
{
   width = 380;
   height = 250;
}

void Maze::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   float gridStartX = 140;
   float gridStartY = 10;
   float sqSize = 28.0f;

   if (x >= gridStartX && x <= gridStartX + 8 * sqSize &&
       y >= gridStartY && y <= gridStartY + 8 * sqSize)
   {
      int col = (int)((x - gridStartX) / sqSize);
      int rowInvert = (int)((y - gridStartY) / sqSize);
      int row = 7 - rowInvert;

      if (col >= 0 && col < 8 && row >= 0 && row < 8)
      {
         int pitch = 48 + row * 8 + col;
         if (mLockedNotes.count(pitch))
            mLockedNotes.erase(pitch);
         else
            mLockedNotes.insert(pitch);

         GenerateChords();
      }
   }
}


void Maze::Poll()
{
}

void Maze::GenerateChords()
{
   mChords.clear();
   mNumChords = mNumChordsIndex + 1;

   for (int i = 0; i < mNumChords; ++i)
   {
      Chord c;
      // Ensure all locked notes are included in every generated chord
      for (int lockedPitch : mLockedNotes)
      {
         c.pitches.push_back(lockedPitch);
      }

      // We still generate a random triad / seventh on top of the locked notes
      // (or you could say the locked notes + generated notes form the chord)
      int basePitch = 48 + (gRandom() % 24); // Random base between C3 and C5
      if (mScaledDegree)
      {
         basePitch = TheScale->MakeDiatonic(basePitch);
      }

      c.pitches.push_back(basePitch);

      // minor or major third (randomly pick +3 or +4, scale will snap it anyway)
      int third = basePitch + (gRandom() % 2 == 0 ? 3 : 4);
      if (mScaledDegree)
         third = TheScale->MakeDiatonic(third);
      c.pitches.push_back(third);

      // fifth
      int fifth = basePitch + 7;
      if (mScaledDegree)
         fifth = TheScale->MakeDiatonic(fifth);
      c.pitches.push_back(fifth);

      // upper harmonics
      float randVal = ofRandom(0, 1.0f);
      int extraNotes = 0;
      if (randVal < mUpperHarmonics)
         extraNotes = 1;
      if (randVal < mUpperHarmonics - 0.2f)
         extraNotes = 2;
      if (randVal < mUpperHarmonics - 0.4f)
         extraNotes = 3;
      if (randVal < mUpperHarmonics - 0.6f)
         extraNotes = 4;
      if (randVal < mUpperHarmonics - 0.8f)
         extraNotes = 5;

      int upperIntervals[] = { 10, 14, 17, 21, 24 };
      for (int u = 0; u < extraNotes && u < 5; ++u)
      {
         int upperNote = basePitch + upperIntervals[u];
         if (mScaledDegree)
            upperNote = TheScale->MakeDiatonic(upperNote);
         c.pitches.push_back(upperNote);
      }

      mChords.push_back(c);
   }
}

void Maze::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   double measureLengthMs = TheTransport->MsPerBar();
   if (measureLengthMs <= 0.0)
      return;

   double msPerChord = measureLengthMs / mNumChords;

   double measureTime = TheTransport->GetMeasureTime(time);
   double curMeasure = floor(measureTime);
   double timeInMeasure = measureTime - curMeasure;
   int curChordIdx = floor(timeInMeasure * mNumChords);

   // Check if we advanced to a new chord slot
   if (curChordIdx != mCurrentChordIndex || curMeasure != mCurrentMeasure)
   {
      // Turn off old notes
      for (int p : mPlayingNotes)
      {
         PlayNoteOutput(NoteMessage(time, p, 0));
      }
      mPlayingNotes.clear();

      mCurrentChordIndex = curChordIdx;
      mCurrentMeasure = curMeasure;

      // Ensure index is valid
      if (mCurrentChordIndex >= 0 && mCurrentChordIndex < mChords.size())
      {
         const Chord& chord = mChords[mCurrentChordIndex];

         double baseTime = time;

         // apply groove (shifting every other chord)
         if (mCurrentChordIndex % 2 != 0)
            baseTime += mGroove * (msPerChord * 0.25f);

         // play new notes
         for (size_t i = 0; i < chord.pitches.size(); ++i)
         {
            double noteTime = baseTime;

            // strum
            noteTime += mStrum * i;

            // humanize timing
            if (mHumaniseTiming > 0.0f)
               noteTime += (ofRandom(-1.0f, 1.0f)) * mHumaniseTiming;

            int velocity = 100;
            // humanize velocity
            if (mHumaniseVelocity > 0.0f)
            {
               velocity += (int)(ofRandom(-1.0f, 1.0f) * 40.0f * mHumaniseVelocity);
               velocity = ofClamp(velocity, 1, 127);
            }

            PlayNoteOutput(NoteMessage(noteTime, chord.pitches[i], velocity));
            mPlayingNotes.push_back(chord.pitches[i]);
         }
      }
   }
}

void Maze::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mScaledDegreeCheckbox)
   {
      // mScaledDegree is auto-updated
      GenerateChords();
   }
}

void Maze::DropdownUpdated(DropdownList* dropdown, int index, double time)
{
   if (dropdown == mNumChordsDropdown)
   {
      mNumChords = index + 1;
      GenerateChords();
   }
}

void Maze::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomiseButton)
   {
      mGrooveSlider->SetValue(ofRandom(0.0f, 1.0f), time);
      mHumaniseVelocitySlider->SetValue(ofRandom(0.0f, 1.0f), time);
      mUpperHarmonicsSlider->SetValue(ofRandom(0.0f, 1.0f), time);
      mStrumSlider->SetValue(ofRandom(0.0f, 100.0f), time);
      mHumaniseTimingSlider->SetValue(ofRandom(0.0f, 50.0f), time);
      mNumChordsDropdown->SetIndex(gRandom() % 16, time, false);
      GenerateChords();
   }
}

void Maze::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Maze::SaveState(FileStreamOut& out)
{
   //NOTE: this used to write out << Name() first instead of the module rev, which broke the
   //framework's SaveState/LoadState protocol - IDrawableModule::LoadModuleSaveStateRev() always
   //reads an int first (expecting GetModuleSaveStateRev()), so reading a string's bytes as that
   //int desynced the stream for the rest of this module AND every module saved after it in the
   //same file. The module's name is already persisted separately via the JSON layout, so it never
   //needed to be saved here at all.
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);

   out << mScaledDegree;
   out << mNumChordsIndex;
   out << mGroove;
   out << mHumaniseVelocity;
   out << mUpperHarmonics;
   out << mStrum;
   out << mHumaniseTiming;

   out << (int)mLockedNotes.size();
   for (int pitch : mLockedNotes)
      out << pitch;
}

void Maze::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   in >> mScaledDegree;
   // mScaledDegreeCheckbox is bound directly to &mScaledDegree, so setting the variable is enough.

   in >> mNumChordsIndex;

   in >> mGroove;
   mGrooveSlider->SetValue(mGroove, 0);
   in >> mHumaniseVelocity;
   mHumaniseVelocitySlider->SetValue(mHumaniseVelocity, 0);
   in >> mUpperHarmonics;
   mUpperHarmonicsSlider->SetValue(mUpperHarmonics, 0);
   in >> mStrum;
   mStrumSlider->SetValue(mStrum, 0);
   in >> mHumaniseTiming;
   mHumaniseTimingSlider->SetValue(mHumaniseTiming, 0);

   mLockedNotes.clear();
   int numLocked = 0;
   in >> numLocked;
   const int kMaxLockedNotes = 128; //can't have more unique locked notes than MIDI pitches; guards
   //against a corrupted/truncated file driving a huge or negative loop here
   LoadStateValidate(numLocked >= 0 && numLocked <= kMaxLockedNotes);
   for (int i = 0; i < numLocked; ++i)
   {
      int pitch;
      in >> pitch;
      mLockedNotes.insert(pitch);
   }

   GenerateChords();
}
