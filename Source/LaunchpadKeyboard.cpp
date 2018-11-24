//
//  LaunchpadKeyboard.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#include "LaunchpadKeyboard.h"
#include "SynthGlobals.h"
#include "IAudioSource.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "Chorder.h"
#include "MidiController.h"
#include "FillSaveDropdown.h"

#define INVALID_PITCH -999
#define CHORD_BUTTON_OFFSET -100
#define CHORD_ENABLE_BUTTON -998
#define CHORD_LATCH_BUTTON -997
#define KEY_LATCH_BUTTON -996

LaunchpadKeyboard::LaunchpadKeyboard()
: mRootNote(4) //4 = E
, mGridController(nullptr)
, mTestKeyHeld(false)
, mLayout(kChromatic)
, mLayoutDropdown(nullptr)
, mOctave(3)
, mOctaveSlider(nullptr)
, mLatch(false)
, mLatchCheckbox(nullptr)
, mCurrentChord(0)
, mHasDisplayer(false)
, mArrangementMode(kFull)
, mArrangementModeDropdown(nullptr)
, mChorder(nullptr)
, mLatchChords(false)
, mLatchChordsCheckbox(nullptr)
, mWasChorderEnabled(false)
, mPreserveChordRoot(true)
, mPreserveChordRootCheckbox(nullptr)
{
   TheScale->AddListener(this);

   TheTransport->AddListener(this, kInterval_8n);

   mHeldChordTones.push_back(0);
   
   SetIsNoteOrigin(true);

   vector<int> chord;
   //triad
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(7);
   mChords.push_back(chord);
   chord.clear();

   //7
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(7);
   chord.push_back(11);
   mChords.push_back(chord);
   chord.clear();

   //6
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(7);
   chord.push_back(9);
   mChords.push_back(chord);
   chord.clear();

   //inv
   chord.push_back(-5);
   chord.push_back(0);
   chord.push_back(4);
   mChords.push_back(chord);
   chord.clear();

   //sus4
   chord.push_back(0);
   chord.push_back(5);
   chord.push_back(7);
   mChords.push_back(chord);
   chord.clear();

   //sus2
   chord.push_back(0);
   chord.push_back(2);
   chord.push_back(7);
   mChords.push_back(chord);
   chord.clear();

   //9
   chord.push_back(0);
   chord.push_back(2);
   chord.push_back(4);
   chord.push_back(7);
   chord.push_back(11);
   mChords.push_back(chord);
   chord.clear();

   //11
   chord.push_back(0);
   chord.push_back(2);
   chord.push_back(4);
   chord.push_back(5);
   chord.push_back(7);
   chord.push_back(11);
   mChords.push_back(chord);
   chord.clear();

   //africa
   chord.push_back(-12);
   chord.push_back(0);
   chord.push_back(4);
   chord.push_back(12);
   chord.push_back(16);
   chord.push_back(28);
   mChords.push_back(chord);
   chord.clear();
}

void LaunchpadKeyboard::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLayoutDropdown = new DropdownList(this,"layout", 6, 22, (int*)(&mLayout));
   mOctaveSlider = new IntSlider(this,"octave",6,40,100,15,&mOctave,0,8);
   mLatchCheckbox = new Checkbox(this,"latch",6,59,&mLatch);
   mArrangementModeDropdown = new DropdownList(this,"arrangement",6,4,((int*)(&mArrangementMode)));
   mLatchChordsCheckbox = new Checkbox(this,"ch.latch",55,59,&mLatchChords);
   mPreserveChordRootCheckbox = new Checkbox(this,"p.root",70,4,&mPreserveChordRoot);
   mGridController = new GridController(this, "grid", 90, 22);
   
   mLayoutDropdown->AddLabel("chromatic", kChromatic);
   mLayoutDropdown->AddLabel("diatonic", kDiatonic);
   mLayoutDropdown->AddLabel("chord indiv", kChordIndividual);
   mLayoutDropdown->AddLabel("chord", kChord);
   mLayoutDropdown->AddLabel("guitar", kGuitar);
   mLayoutDropdown->AddLabel("septatonic", kSeptatonic);
   
   mArrangementModeDropdown->AddLabel("full", kFull);
   mArrangementModeDropdown->AddLabel("five", kFive);
   mArrangementModeDropdown->AddLabel("six", kSix);
}

LaunchpadKeyboard::~LaunchpadKeyboard()
{
   TheScale->RemoveListener(this);
   TheTransport->RemoveListener(this);
}

void LaunchpadKeyboard::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   bool bOn = velocity > 0;
   int pitch = GridToPitch(x,y);
   
   if (pitch == INVALID_PITCH)
   {
      ReleaseNoteFor(x, y);
      return;
   }
   if (pitch == CHORD_ENABLE_BUTTON)
   {
      if (bOn && mChorder)
         mChorder->SetEnabled(!mChorder->Enabled());
      return;
   }
   if (pitch == CHORD_LATCH_BUTTON)
   {
      if (bOn)
      {
         mLatchChords = !mLatchChords;
         UpdateLights();
      }
      return;
   }
   if (pitch == KEY_LATCH_BUTTON)
   {
      if (bOn)
      {
         mLatch = !mLatch;
         if (!mLatch)
            mNoteOutput.Flush();
         UpdateLights();
      }
      return;
   }
   if (pitch < 0)
   {
      HandleChordButton(pitch, bOn);

      UpdateLights();

      return;
   }
   if (!mLatch && mLayout != kChord)
   {
      //handled below
   }
   else if (mLatch)
   {
      if (bOn) //only presses matter in latch, not releases
      {
         int currentPitch = -1;
         if (mHeld.size() == 1)  //we should only have one at a time in latch mode
            currentPitch = mHeld.begin()->mPitch;
         mHeld.clear();
         mNoteOutput.Flush();

         if (currentPitch == pitch)
         {
            bOn = false;   //pressed the note again, this is a note-off
         }
         else
         {
            PressedNoteFor(x, y, pitch);
            bOn = true;
         }
      }
      else
      {
         return;
      }
   }

   if (mLayout == kChord)
   {
      if (bOn)
      {
         mHeld.clear();
         PressedNoteFor(x, y, x+y*8);
         mNoteOutput.Flush();
         for (int i=0; i<mChords[x].size(); ++i)
            PlayNoteOutput(gTime, TheScale->MakeDiatonic(pitch+mChords[x][i]), 127*velocity, -1);
      }
      else if (x+y*8 == mHeld.begin()->mPitch)
      {
         mHeld.clear();
         mNoteOutput.Flush();
      }
   }
   else
   {
      if (bOn)
      {
         PlayNoteOutput(gTime, pitch, 127*velocity, -1);
         PressedNoteFor(x,y,pitch);
      }
      else
      {
         ReleaseNoteFor(x,y);
      }
   }

   if (!mHasDisplayer)  //we don't have a displayer, handle it ourselves
   {
      mCurrentNotes.clear();
      for (auto i=mHeld.begin(); i != mHeld.end(); ++i)
         mCurrentNotes.push_back(i->mPitch);

      UpdateLights();

      int lowestPitch = 999;
      for (auto iter = mHeld.begin(); iter != mHeld.end(); ++iter)
      {
         if (iter->mPitch < lowestPitch)
            lowestPitch = iter->mPitch;
      }
      lowestPitch -= TheScale->GetTet();

      if (bOn > 0 && mHeld.size() > 0)
         gVizFreq = MAX(1,TheScale->PitchToFreq(lowestPitch));
   }
}

void LaunchpadKeyboard::PressedNoteFor(int x, int y, int pitch)
{
   HeldButton held;
   held.mX = x;
   held.mY = y;
   held.mPitch = pitch;
   mHeld.push_back(held);
}

void LaunchpadKeyboard::ReleaseNoteFor(int x, int y)
{
   for (auto i = mHeld.begin(); i != mHeld.end(); ++i)
   {
      if (i->mX == x && i->mY == y)
      {
         PlayNoteOutput(gTime, i->mPitch, 0, -1);
         mHeld.erase(i);
         return;
      }
   }
}

void LaunchpadKeyboard::HandleChordButton(int pitch, bool bOn)
{
   int chordTone = pitch - CHORD_BUTTON_OFFSET;

   if (mPreserveChordRoot && chordTone == 0)  //root always pressed
      return;

   if (!mLatchChords)
   {
      if (bOn)
      {
         mHeldChordTones.push_back(chordTone);
         if (mChorder)
            mChorder->AddTone(chordTone);
      }
      else
      {
         mHeldChordTones.remove(chordTone);
         if (mChorder)
            mChorder->RemoveTone(chordTone);
      }
   }
   else if (bOn)  //latch only pays attention to presses
   {
      if (!ListContains(chordTone,mHeldChordTones))
      {
         mHeldChordTones.push_back(chordTone);
         if (mChorder)
            mChorder->AddTone(chordTone);
      }
      else
      {
         mHeldChordTones.remove(chordTone);
         if (mChorder)
            mChorder->RemoveTone(chordTone);
      }
   }
}

bool LaunchpadKeyboard::IsChordButtonPressed(int pitch)
{
   int chordTone = pitch - CHORD_BUTTON_OFFSET;
   
   if (mPreserveChordRoot && chordTone == 0)  //root always pressed
      return true;

   return ListContains(chordTone,mHeldChordTones);
}

void LaunchpadKeyboard::OnTimeEvent(int samplesTo)
{
}

void LaunchpadKeyboard::DisplayNote(int pitch, int velocity)
{
   if (velocity > 0)
      mCurrentNotes.push_back(pitch);
   else
      mCurrentNotes.remove(pitch);

   UpdateLights();
}

void LaunchpadKeyboard::DrawModule()
{
   if (mChorder)
      DrawConnection(mChorder);
   if (Minimized() || IsVisible() == false)
      return;
   mLayoutDropdown->Draw();
   mOctaveSlider->Draw();
   mLatchCheckbox->Draw();
   mLatchChordsCheckbox->Draw();
   mArrangementModeDropdown->Draw();
   mPreserveChordRootCheckbox->Draw();
   mGridController->Draw();
}

int LaunchpadKeyboard::GridToPitch(int x, int y)
{
   y = 7-y;
   if (mArrangementMode == kSix)
   {
      if (x < 2)
      {
         return GridToPitchChordSection(x,y);
      }
      else
      {
         x -= 2;
      }
      return TheScale->ScaleRoot() + x + 6*y + TheScale->GetTet()*mOctave;
   }
   if (mLayout == kChromatic)
   {
      if (mArrangementMode == kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x,y);
         }
         else
         {
            x -= 3;
         }
      }
      return mRootNote + x + 5*y + TheScale->GetTet()*mOctave;
   }
   if (mLayout == kGuitar)
   {
      return mRootNote + x + 5*y + TheScale->GetTet()*mOctave + (y>=4 ? -1 : 0);
   }
   else if (mLayout == kDiatonic)
   {
      if (mArrangementMode == kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x,y);
         }
         else if (x == 3 || x == 7)
         {
            return INVALID_PITCH;
         }
         else
         {
            x -= 4;
         }
      }
      return TheScale->GetPitchFromTone(x + 3*y) + TheScale->GetTet()*(mRootNote/TheScale->GetTet()) + TheScale->GetTet()*mOctave;
   }
   else if (mLayout == kChordIndividual)
   {
      int note = x%mChords[mCurrentChord].size();
      int oct = x/mChords[mCurrentChord].size();
      return TheScale->MakeDiatonic(TheScale->GetPitchFromTone(y) + mChords[mCurrentChord][note]) + TheScale->GetTet()*(mOctave+oct);
   }
   else if (mLayout == kChord)
   {
      if (x < mChords.size())
         return TheScale->GetPitchFromTone(y) + TheScale->GetTet()*mOctave;
      else
         return INVALID_PITCH;
   }
   else if (mLayout == kSeptatonic)
   {
      if (mArrangementMode == kFive)
      {
         if (x < 3)
         {
            return GridToPitchChordSection(x,y);
         }
         else if (x == 3)
         {
            if (TheScale->NumPitchesInScale() == 7)   //septatonic scales only
            {
               if (y%2 == 0)  // 7 or maj7
               {
                  int nonDiatonic = TheScale->GetPitchFromTone(0) - 1 + TheScale->GetTet()*(mRootNote/TheScale->GetTet()) + TheScale->GetTet()*(mOctave+y/2);
                  if (TheScale->IsInScale(nonDiatonic))
                     --nonDiatonic;
                  return nonDiatonic;
               }
               if (y%2 == 1)  // 4 or sharp 4
               {
                  int nonDiatonic = TheScale->GetPitchFromTone(4) - 1 + TheScale->GetTet()*(mRootNote/TheScale->GetTet()) + TheScale->GetTet()*(mOctave+y/2);
                  if (TheScale->IsInScale(nonDiatonic))
                     --nonDiatonic;
                  return nonDiatonic;
               }
            }
            return INVALID_PITCH;
         }
         else
         {
            x -= 4;
         }
      }
      
      int numPitchesInScale = TheScale->NumPitchesInScale();
      if (numPitchesInScale > 8)
         return INVALID_PITCH;
      
      int pos = x + 4*y;
      int set = pos/8;
      int tone = pos - set*(8-numPitchesInScale);// + TheScale->GetScaleDegree(); add this for chord following
      if (pos % 8 >= numPitchesInScale)
         return INVALID_PITCH;
      
      return TheScale->GetPitchFromTone(tone) + TheScale->GetTet()*(mRootNote/TheScale->GetTet()) + TheScale->GetTet()*mOctave;
   }
   assert(false);
   return 0;
}

int LaunchpadKeyboard::GridToPitchChordSection(int x, int y)
{
   int numPitchesInScale = TheScale->NumPitchesInScale();
   
   if (y<7 && y<numPitchesInScale)
   {
      return CHORD_BUTTON_OFFSET + (numPitchesInScale*(x-1)) + y;
   }
   else if (y==7)
   {
      if (x == 0)
         return CHORD_ENABLE_BUTTON;
      if (x == 1)
         return CHORD_LATCH_BUTTON;
      if (x == 2)
         return KEY_LATCH_BUTTON;
   }
   
   return INVALID_PITCH;
}

void LaunchpadKeyboard::UpdateLights(bool force)
{
   for (int x=0; x<8; ++x)
   {
      for (int y=0; y<8; ++y)
      {
         int pitch = GridToPitch(x,y);
         bool inScale = TheScale->MakeDiatonic(pitch) == pitch;
         bool isRoot = pitch%TheScale->GetTet() == TheScale->ScaleRoot();
         bool isHeld = false;
         bool isSameOctave = false;
         bool isInPentatonic = pitch >= 0 && TheScale->IsInPentatonic(pitch);
         bool isChordButton = pitch != INVALID_PITCH && pitch < 0;
         bool isPressedChordButton = isChordButton && IsChordButtonPressed(pitch);
         bool isChorderEnabled = mChorder && mChorder->Enabled();

         if (mLayout == kChord)
         {
            isHeld = mCurrentNotes.size() && (x+y*8 == *(mCurrentNotes.begin()));
            isSameOctave = false;
         }
         else
         {
            for (list<int>::iterator iter = mCurrentNotes.begin(); iter != mCurrentNotes.end(); ++iter)
            {
               if (*iter == pitch)
                  isHeld = true;
               if (*iter%TheScale->GetTet() == pitch % TheScale->GetTet())
                  isSameOctave = true;
            }
         }
         
         GridColor color;
         if (pitch == INVALID_PITCH)
         {
            color = kGridColorOff;
         }
         else if (pitch == CHORD_ENABLE_BUTTON)
         {
            if (mChorder && mChorder->Enabled())
               color = kGridColor1Bright;
            else
               color = kGridColor1Dim;
         }
         else if (pitch == CHORD_LATCH_BUTTON)
         {
            if (mLatchChords)
               color = kGridColor3Bright;
            else
               color = kGridColor3Dim;
         }
         else if (pitch == KEY_LATCH_BUTTON)
         {
            if (mLatch)
               color = kGridColor1Bright;
            else
               color = kGridColor1Dim;
         }
         else if (isPressedChordButton && isChorderEnabled)
         {
            color = kGridColor2Bright;
         }
         else if (isChordButton && !isChorderEnabled)
         {
            color = kGridColor1Dim;
         }
         else if (isChordButton)
         {
            color = kGridColor2Dim;
         }
         else if (isHeld)
         {
            color = kGridColor1Bright;
         }
         /*else if (isSameOctave)
         {
            color = kGridColor3Dim;
         }*/
         else if (isRoot)
         {
            color = kGridColor2Bright;
         }
         else if (isInPentatonic)
         {
            color = kGridColor3Bright;
         }
         else if (inScale)
         {
            color = kGridColor3Dim;
         }
         else
         {
            color = kGridColorOff;
         }
         
         if (mGridController)
            mGridController->SetLight(x, y, color, force);
      }
   }
}

void LaunchpadKeyboard::OnControllerPageSelected()
{
   UpdateLights();
}

void LaunchpadKeyboard::OnScaleChanged()
{
   UpdateLights();
}

void LaunchpadKeyboard::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   if (key == 'e' && !mTestKeyHeld && GetKeyModifiers() == kModifier_None)
   {
      mTestKeyHeld = true;
      OnGridButton(3,3,1,mGridController);
   }
}

void LaunchpadKeyboard::KeyReleased(int key)
{
   if (key == 'e' && GetKeyModifiers() == kModifier_None)
   {
      mTestKeyHeld = false;
      OnGridButton(3,3,0,mGridController);
   }
}

void LaunchpadKeyboard::Poll()
{
   bool chorderEnabled = mChorder && mChorder->Enabled();
   if (chorderEnabled != mWasChorderEnabled)
   {
      mWasChorderEnabled = chorderEnabled;
      UpdateLights();
   }
}

void LaunchpadKeyboard::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mPreserveChordRootCheckbox)
   {
      if (!ListContains(0,mHeldChordTones))
      {
         mHeldChordTones.push_back(0);
         if (mChorder)
            mChorder->AddTone(0);
         UpdateLights();
      }
   }
   if (checkbox == mLatchCheckbox)
   {
      if (!mLatch)
         mNoteOutput.Flush();
   }
}

void LaunchpadKeyboard::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mOctaveSlider)
   {
      mHeld.clear();
      mNoteOutput.Flush();
      UpdateLights();
   }
}

void LaunchpadKeyboard::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void LaunchpadKeyboard::Exit()
{
   IDrawableModule::Exit();
   if (mGridController)
      mGridController->ResetLights();
}

void LaunchpadKeyboard::DropdownUpdated(DropdownList* list, int oldVal)
{
   UpdateLights();
}

void LaunchpadKeyboard::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("chorder",moduleInfo,"",FillDropdown<Chorder*>);
   mModuleSaveData.LoadEnum<ArrangementMode>("arrangement", moduleInfo, kFull, mArrangementModeDropdown);
   mModuleSaveData.LoadEnum<LaunchpadLayout>("layout", moduleInfo, kChromatic, mLayoutDropdown);

   SetUpFromSaveData();
}

void LaunchpadKeyboard::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetChorder(dynamic_cast<Chorder*>(TheSynth->FindModule(mModuleSaveData.GetString("chorder"),false)));
   mArrangementMode = mModuleSaveData.GetEnum<ArrangementMode>("arrangement");
   mLayout = mModuleSaveData.GetEnum<LaunchpadLayout>("layout");
}


