//
//  NoteStepSequencer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/3/13.
//
//

#include "NoteStepSequencer.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "NoteStepSequencer.h"
#include "LaunchpadInterpreter.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"

NoteStepSequencer::NoteStepSequencer()
: mInterval(kInterval_8n)
, mArpIndex(-1)
, mIntervalSelector(nullptr)
, mNumSteps(8)
, mLength(8)
, mLengthSlider(nullptr)
, mGrid(nullptr)
, mLastPitch(-1)
, mLastVel(0)
, mOctave(0)
, mOctaveSlider(nullptr)
, mNoteMode(kNoteMode_Scale)
, mNoteModeSelector(nullptr)
, mNoteRange(15)
, mShowStepControls(false)
, mRowOffset(0)
, mSetLength(false)
, mController(nullptr)
, mShiftBackButton(nullptr)
, mShiftForwardButton(nullptr)
, mLastNoteLength(1)
, mLastNoteStartTime(0)
, mLastNoteEndTime(0)
, mAlreadyDidNoteOff(false)
, mRandomizePitchButton(nullptr)
, mRandomizeLengthButton(nullptr)
, mRandomizeVelocityButton(nullptr)
, mLoopResetPoint(0)
, mLoopResetPointSlider(nullptr)
, mHasExternalPulseSource(false)
{
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
   TheTransport->AddAudioPoller(this);
   
   for (int i=0;i<NSS_MAX_STEPS;++i)
   {
      mVels[i] = 127;
      mNoteLengths[i] = 1;
   }
   
   RandomizePitches(true);
   
   SetIsNoteOrigin(true);
   
   TheScale->AddListener(this);
}

void NoteStepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",75,2,(int*)(&mInterval));
   mLengthSlider = new IntSlider(this,"length",77,20,98,15,&mLength,1,mNumSteps);
   mGrid = new UIGrid(5,55,200,80,8,24, this);
   mVelocityGrid = new UIGrid(5,117,200,45,8,1, this);
   mOctaveSlider = new IntSlider(this,"octave",166,2,53,15,&mOctave,-2,4);
   mNoteModeSelector = new DropdownList(this,"notemode",5,20,(int*)(&mNoteMode));
   mShiftBackButton = new ClickButton(this,"<",130,2);
   mShiftForwardButton = new ClickButton(this,">",145,2);
   mRandomizePitchButton = new ClickButton(this,"pitch",140,38);
   mRandomizeLengthButton = new ClickButton(this,"len",-1,-1);
   mRandomizeVelocityButton = new ClickButton(this,"vel",-1,-1);
   mLoopResetPointSlider = new IntSlider(this,"loop reset",-1,-1,100,15,&mLoopResetPoint,0,mLength);
   
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      mToneDropdowns[i] = new DropdownList(this,("tone"+ofToString(i)).c_str(),-1,-1,&(mTones[i]),40);
      mToneDropdowns[i]->SetDrawTriangle(false);
      mToneDropdowns[i]->SetShowing(false);
      mVelocitySliders[i] = new IntSlider(this,("vel"+ofToString(i)).c_str(),-1,-1,30,15,&mVels[i],0,127);
      mVelocitySliders[i]->SetShowName(false);
      mVelocitySliders[i]->SetShowing(false);
      mLengthSliders[i] = new FloatSlider(this,("len"+ofToString(i)).c_str(),-1,-1,30,15,&mNoteLengths[i],0.01f,1,1);
      mLengthSliders[i]->SetShowName(false);
      mLengthSliders[i]->SetShowing(false);
   }
   SetUpStepControls();
   
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("none", kInterval_None);
   
   mNoteModeSelector->AddLabel("scale", kNoteMode_Scale);
   mNoteModeSelector->AddLabel("chromatic", kNoteMode_Chromatic);
   mNoteModeSelector->AddLabel("5ths", kNoteMode_Fifths);
   
   mGrid->SetSingleColumnMode(true);
   mGrid->SetFlip(true);
   mGrid->SetListener(this);
   mGrid->SetGridMode(UIGrid::kHorislider);
   mGrid->SetClickClearsToZero(false);
   mVelocityGrid->SetGridMode(UIGrid::kMultislider);
   mVelocityGrid->SetClickClearsToZero(false);
   mVelocityGrid->SetListener(this);
   
   mLoopResetPointSlider->SetShowing(mHasExternalPulseSource);
   
   mRandomizeLengthButton->PositionTo(mRandomizePitchButton, kAnchor_Right);
   mRandomizeVelocityButton->PositionTo(mRandomizeLengthButton, kAnchor_Right);
}

NoteStepSequencer::~NoteStepSequencer()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
   TheScale->RemoveListener(this);
}

void NoteStepSequencer::SetMidiController(string name)
{
   if (mController)
      mController->RemoveListener(this);
   
   mController = TheSynth->FindMidiController(name);
   if (mController)
      mController->AddListener(this, 0);
   
   UpdateLights();
}

void NoteStepSequencer::Init()
{
   IDrawableModule::Init();
   
   SyncGridToSeq();
}

void NoteStepSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mLoopResetPointSlider->SetShowing(mHasExternalPulseSource);
   
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mOctaveSlider->Draw();
   mNoteModeSelector->Draw();
   mShiftBackButton->Draw();
   mShiftForwardButton->Draw();
   mRandomizePitchButton->Draw();
   mRandomizeLengthButton->Draw();
   mRandomizeVelocityButton->Draw();
   mLoopResetPointSlider->Draw();
   
   mGrid->Draw();
   mVelocityGrid->Draw();
   
   ofPushStyle();
   ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
   for (int i=0; i<mGrid->GetRows(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(0, i) + mGrid->GetPosition(true);
      float scale = MIN(mGrid->IClickable::GetDimensions().y / mGrid->GetRows(), 20);
      DrawTextNormal(NoteName(RowToPitch(i),false,true) + "("+ ofToString(RowToPitch(i)) + ")", pos.x - 5, pos.y - (scale/8), scale);
   }
   ofPopStyle();
   
   DrawTextLeftJustify("random:", 138, 50);
   
   ofPushStyle();
   ofFill();
   float gridX, gridY, gridW, gridH;
   mGrid->GetPosition(gridX, gridY, true);
   gridW = mGrid->GetWidth();
   gridH = mGrid->GetHeight();
   float boxHeight = (float(gridH)/mNoteRange);
   float boxWidth = (float(gridW)/mGrid->GetCols());
   
   for (int i=0; i<mGrid->GetCols()-1; ++i)
   {
      if (mNoteLengths[i] == 1 && mTones[i] == mTones[i+1] && mVels[i] > mVels[i+1])
      {
         ofSetColor(255,255,255,255);
         ofFill();
         float y = gridY + gridH - mTones[i]*boxHeight;
         ofRect(gridX + boxWidth * i+1, y-boxHeight+1, boxWidth*1.5f-2, boxHeight-2);
      }
   }
   
   for (int i=0;i<mNoteRange;++i)
   {
      if (RowToPitch(i)%TheScale->GetTet() == TheScale->ScaleRoot()%TheScale->GetTet())
         ofSetColor(0,255,0,80);
      else if (RowToPitch(i)%TheScale->GetTet() == (TheScale->ScaleRoot()+7)%TheScale->GetTet())
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
      if (mVels[i] == 0 || i >= mLength)
      {
         ofSetColor(0,0,0,100);
         ofFill();
         ofRect(gridX + boxWidth * i, gridY, boxWidth, gridH);
      }
   }
   
   float controlYPos = gridY+gridH+mVelocityGrid->GetHeight();
   if (mLoopResetPointSlider->IsShowing())
      controlYPos += 19;
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      if (i < mLength)
      {
         mToneDropdowns[i]->SetShowing(mShowStepControls);
         mToneDropdowns[i]->SetPosition(gridX+boxWidth*i, controlYPos);
         mToneDropdowns[i]->SetWidth(boxWidth);
         mToneDropdowns[i]->Draw();
         
         mVelocitySliders[i]->SetShowing(mShowStepControls);
         mVelocitySliders[i]->SetPosition(gridX+boxWidth*i, controlYPos+17);
         mVelocitySliders[i]->SetDimensions(boxWidth, 15);
         mVelocitySliders[i]->Draw();
         
         mLengthSliders[i]->SetShowing(mShowStepControls);
         mLengthSliders[i]->SetPosition(gridX+boxWidth*i, controlYPos+32);
         mLengthSliders[i]->SetDimensions(boxWidth, 15);
         mLengthSliders[i]->Draw();
      }
      else
      {
         mToneDropdowns[i]->SetShowing(false);
         mVelocitySliders[i]->SetShowing(false);
         mLengthSliders[i]->SetShowing(false);
      }
   }
   
   ofPopStyle();
}

void NoteStepSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mGrid->TestClick(x,y,right);
   mVelocityGrid->TestClick(x, y, right);
}

void NoteStepSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool NoteStepSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool NoteStepSequencer::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   mGrid->NotifyMouseScrolled(x,y,scrollX,scrollY);
   mVelocityGrid->NotifyMouseScrolled(x,y,scrollX,scrollY);
   return false;
}

void NoteStepSequencer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteStepSequencer::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
      for (int i=0; i<mGrid->GetCols(); ++i)
      {
         for (int j=0; j<mGrid->GetRows(); ++j)
         {
            float val = mGrid->GetVal(i,j);
            if (val > 0)
            {
               mTones[i] = j;
               mNoteLengths[i] = val;
               break;
            }
         }
      }
   }
   if (grid == mVelocityGrid)
   {
      for (int i=0; i<mVelocityGrid->GetCols(); ++i)
         mVels[i] = mVelocityGrid->GetVal(i,0) * 127;
   }
}

int NoteStepSequencer::RowToPitch(int row)
{
   row += mRowOffset;
   
   int numPitchesInScale = TheScale->NumPitchesInScale();
   switch (mNoteMode)
   {
      case kNoteMode_Scale:
         return TheScale->GetPitchFromTone(row+(mOctave+3)*numPitchesInScale+TheScale->GetScaleDegree());
      case kNoteMode_Chromatic:
         return row + (mOctave+2) * TheScale->GetTet();
      case kNoteMode_Fifths:
      {
         int oct = (row/2)*numPitchesInScale;
         bool isFifth = row%2 == 1;
         int fifths = oct;
         if (isFifth)
            fifths += 4;
         return TheScale->GetPitchFromTone(fifths+(mOctave+3)*numPitchesInScale+TheScale->GetScaleDegree());

      }
   }
   return row;
}

int NoteStepSequencer::PitchToRow(int pitch)
{
   for (int i=0; i<mGrid->GetRows(); ++i)
   {
      if (pitch == RowToPitch(i))
         return i;
   }
   return -1;
}

void NoteStepSequencer::SetStep(int index, int pitch, int velocity, float length)
{
   if (index >= 0 && index < NSS_MAX_STEPS)
   {
      mTones[index] = PitchToRow(pitch);
      mVels[index] = ofClamp(velocity, 0, 127);
      mNoteLengths[index] = length;
      SyncGridToSeq();
   }
}

void NoteStepSequencer::OnTransportAdvanced(float amount)
{
   PROFILER(NoteStepSequencer);
   
   ComputeSliders(0);
   
   if (mLastNoteLength < 1 && !mAlreadyDidNoteOff)
   {
      if (gTime > mLastNoteEndTime)
      {
         PlayNoteOutput(gTime, mLastPitch, 0);
         mAlreadyDidNoteOff = true;
      }
   }
}

void NoteStepSequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;
   
   Step(time, velocity, flags);
}

void NoteStepSequencer::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void NoteStepSequencer::Step(double time, float velocity, int pulseFlags)
{
   if (!mEnabled)
      return;
   
   int direction = 1;
   if (pulseFlags & kPulseFlag_Backward)
      direction = -1;
   
   mArpIndex += direction;
   if (direction > 0 && mArpIndex >= mLength)
      mArpIndex -= (mLength - mLoopResetPoint);
   if (direction < 0 && mArpIndex < mLoopResetPoint)
      mArpIndex += (mLength - mLoopResetPoint);
   
   if (pulseFlags & kPulseFlag_Reset)
      mArpIndex = 0;
   else if (pulseFlags & kPulseFlag_Random)
      mArpIndex = rand() % mLength;
   
   if (!mHasExternalPulseSource || (pulseFlags & kPulseFlag_SyncToTransport))
   {
      int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop()/TheTransport->GetTimeSigBottom();
      int numMeasures = ceil(float(mLength) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      int step = (TheTransport->GetQuantized(time, mInterval) + measure * stepsPerMeasure) % mLength;
      mArpIndex = step;
   }
   
   int offPitch = -1;
   if (mLastPitch >= 0 && !mAlreadyDidNoteOff)
   {
      offPitch = mLastPitch;
   }
   
   int current = mTones[mArpIndex];
   if (mVels[mArpIndex] <= 1)
   {
      mLastPitch = -1;
      mLastVel = 0;
   }
   else
   {
      int outPitch = RowToPitch(current);
      
      if (mLastPitch == outPitch && !mAlreadyDidNoteOff)   //same note, play noteoff first
      {
         PlayNoteOutput(time, mLastPitch, 0, -1);
         offPitch = -1;
      }
      if (mVels[mArpIndex] > 1)
      {
         PlayNoteOutput(time, outPitch, mVels[mArpIndex] * velocity, -1);
         mLastPitch = outPitch;
         mLastVel = mVels[mArpIndex];
         mLastNoteLength = mNoteLengths[mArpIndex];
         mLastNoteStartTime = time;
         mLastNoteEndTime = time + mLastNoteLength * TheTransport->GetDuration(mInterval);
         mAlreadyDidNoteOff = false;
      }
   }
   
   if (offPitch != -1)
   {
      PlayNoteOutput(time, offPitch, 0, -1);
      if (offPitch == mLastPitch)
      {
         mLastPitch = -1;
         mLastVel = 0;
      }
   }
   
   mGrid->SetHighlightCol(mArpIndex);
   mVelocityGrid->SetHighlightCol(mArpIndex);
   
   UpdateLights();
}

void NoteStepSequencer::UpdateLights()
{
   if (mController)
   {
      for (int i=0;i<NSS_MAX_STEPS;++i)
      {
         int button = StepToButton(i);
         int color = 0;
         if (i<mLength)
         {
            if (i == mArpIndex)
            {
               color = LaunchpadInterpreter::LaunchpadColor(0,3);
            }
            else if (mVels[i] > 1)
            {
               int level = (mVels[i] / 50) + 1;
               color = LaunchpadInterpreter::LaunchpadColor(level,level);
            }
            else
            {
               color = LaunchpadInterpreter::LaunchpadColor(1,0);
            }
         }
         mController->SendNote(0, button, color);
      }
   }
}

int NoteStepSequencer::ButtonToStep(int button)
{
   if ((button >= 9 && button <= 12) ||
       (button >= 25 && button <= 28))
   {
      return button <= 12 ? (button - 9) : (button - 21);
   }
   return -1;
}

int NoteStepSequencer::StepToButton(int step)
{
   return step < 4 ? (step + 9) : (step + 21);
}

float NoteStepSequencer::ExtraWidth() const
{
   return 10;
}

float NoteStepSequencer::ExtraHeight() const
{
   float height = 103;
   if (mLoopResetPointSlider->IsShowing())
      height += 17;
   if (mShowStepControls)
      height += 17 * 3;
   return height;
}

void NoteStepSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + ExtraWidth();
   height = mGrid->GetHeight() + ExtraHeight();
}

void NoteStepSequencer::Resize(float w, float h)
{
   mGrid->SetDimensions(MAX(w - ExtraWidth(), 210), MAX(h - ExtraHeight(), 80));
   UpdateVelocityGridPos();
}

void NoteStepSequencer::UpdateVelocityGridPos()
{
   mVelocityGrid->SetDimensions(mGrid->GetWidth(), 45);
   float gridX,gridY;
   mGrid->GetPosition(gridX, gridY, true);
   mVelocityGrid->SetPosition(gridX, gridY + mGrid->GetHeight());
   mLoopResetPointSlider->PositionTo(mVelocityGrid, kAnchor_Below);
   mLoopResetPointSlider->SetDimensions(mVelocityGrid->GetWidth(), 15);
}

void NoteStepSequencer::OnMidiNote(MidiNote& note)
{
   int step = ButtonToStep(note.mPitch);
   if (step != -1 && note.mVelocity > 0)
   {
      if (mSetLength)
      {
         mLength = step+1;
         UpdateLights();
      }
      else
      {
         mArpIndex = step-1;
      }
   }
}

void NoteStepSequencer::OnMidiControl(MidiControl& control)
{
   if (control.mControl >= 21 && control.mControl <= 28)
   {
      int step = control.mControl - 21;
      mTones[step] = MIN(control.mValue/127.0f * mNoteRange, mNoteRange-1);
      SyncGridToSeq();
   }
   
   if (control.mControl >= 41 && control.mControl <= 48)
   {
      int step = control.mControl - 41;
      if (control.mValue >= 1)
         mVels[step] = control.mValue;
      SyncGridToSeq();
      UpdateLights();
   }
   
   if (control.mControl == 115)
   {
      mSetLength = control.mValue > 0;
      if (mController)
         mController->SendCC(0, 116, control.mValue);
   }
}

void NoteStepSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      int tet = TheScale->GetTet();
      mOctave = (pitch - TheScale->ScaleRoot()) / tet - 3;
      
      if (mNoteMode == kNoteMode_Scale)
         mRowOffset = TheScale->GetToneFromPitch(pitch) % TheScale->NumPitchesInScale();
      else if (mNoteMode == kNoteMode_Chromatic)
         mRowOffset = ((pitch % tet) - (TheScale->ScaleRoot() % tet) + tet) % tet;
      else if (mNoteMode == kNoteMode_Fifths)
         mRowOffset = 0;
   }
}

void NoteStepSequencer::ShiftSteps(int amount)
{
   int newTones[NSS_MAX_STEPS];
   int newVels[NSS_MAX_STEPS];
   float newLengths[NSS_MAX_STEPS];
   memcpy(newTones, mTones, NSS_MAX_STEPS*sizeof(int));
   memcpy(newVels, mVels, NSS_MAX_STEPS*sizeof(int));
   memcpy(newLengths, mNoteLengths, NSS_MAX_STEPS*sizeof(float));
   for (int i=0; i<mLength; ++i)
   {
      int dest = (i + mLength + amount) % mLength;
      newTones[dest] = mTones[i];
      newVels[dest] = mVels[i];
      newLengths[dest] = mNoteLengths[i];
   }
   memcpy(mTones, newTones, NSS_MAX_STEPS*sizeof(int));
   memcpy(mVels, newVels, NSS_MAX_STEPS*sizeof(int));
   memcpy(mNoteLengths, newLengths, NSS_MAX_STEPS*sizeof(float));
   SyncGridToSeq();
}

void NoteStepSequencer::ButtonClicked(ClickButton* button)
{
   if (button == mShiftBackButton)
      ShiftSteps(-1);
   if (button == mShiftForwardButton)
      ShiftSteps(1);
   if (button == mRandomizePitchButton)
   {
      RandomizePitches(GetKeyModifiers() & kModifier_Shift);
      SyncGridToSeq();
   }
   if (button == mRandomizeLengthButton)
   {
      for (int i=0; i<mNumSteps; ++i)
         mNoteLengths[i] = ofClamp(ofRandom(2), FLT_EPSILON, 1);
      SyncGridToSeq();
   }
   if (button == mRandomizeVelocityButton)
   {
      for (int i=0; i<mNumSteps; ++i)
      {
         switch (rand() % 4)
         {
            case 0:
               mVels[i] = 0;
               break;
            case 1:
            case 2:
               mVels[i] = 80;
               break;
            case 3:
               mVels[i] = 127;
               break;
         }
      }
      SyncGridToSeq();
   }
}

void NoteStepSequencer::RandomizePitches(bool fifths)
{
   if (fifths)
   {
      for (int i=0; i<mNumSteps; ++i)
      {
         switch (rand() % 5)
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
   else
   {
      for (int i=0; i<mNumSteps; ++i)
         mTones[i] = rand() % mNoteRange;
   }
}

void NoteStepSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      TheTransport->UpdateListener(this, mInterval);
   if (list == mNoteModeSelector)
   {
      if (mNoteMode != oldVal)
         mRowOffset = 0;
      SetUpStepControls();
   }
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      if (list == mToneDropdowns[i])
         SyncGridToSeq();
   }
}

void NoteStepSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      if (slider == mLengthSliders[i])
         SyncGridToSeq();
   }
}

void NoteStepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLoopResetPointSlider || slider == mLengthSlider)
      mLoopResetPoint = MIN(mLoopResetPoint, mLength-1);
   if (slider == mLengthSlider)
      SyncGridToSeq();
   if (slider == mOctaveSlider)
      SetUpStepControls();
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      if (slider == mVelocitySliders[i])
         SyncGridToSeq();
   }
}

void NoteStepSequencer::SyncGridToSeq()
{
   mGrid->Clear();
   mVelocityGrid->Clear();
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      if (mTones[i] < 0)
         continue;
      
      mGrid->SetVal(i,mTones[i],mNoteLengths[i],false);
      mVelocityGrid->SetVal(i, 0, mVels[i]/127.0f, false);
   }
   mGrid->SetGrid(mNumSteps, mNoteRange);
   mVelocityGrid->SetGrid(mNumSteps, 1);
}

void NoteStepSequencer::OnScaleChanged()
{
   SetUpStepControls();
}

void NoteStepSequencer::SetUpStepControls()
{
   if (TheSynth->IsLoadingModule())
      return;
   
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      mToneDropdowns[i]->Clear();
      for (int j=14; j>=0; --j)
         mToneDropdowns[i]->AddLabel(NoteName(RowToPitch(j), false, true), j);
   }
}

void NoteStepSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["gridwidth"] = mGrid->GetWidth();
   moduleInfo["gridheight"] = mGrid->GetHeight();
}

void NoteStepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);
   mModuleSaveData.LoadInt("gridwidth", moduleInfo, 210, 210, 2000, true);
   mModuleSaveData.LoadInt("gridheight", moduleInfo, 120, 80, 2000, true);
   mModuleSaveData.LoadInt("gridrows", moduleInfo, 15, 1, 127);
   mModuleSaveData.LoadInt("gridsteps", moduleInfo, 8, 1, NSS_MAX_STEPS);
   mModuleSaveData.LoadBool("stepcontrols", moduleInfo, false);

   SetUpFromSaveData();
}

void NoteStepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetMidiController(mModuleSaveData.GetString("controller"));
   mGrid->SetDimensions(mModuleSaveData.GetInt("gridwidth"), mModuleSaveData.GetInt("gridheight"));
   mNoteRange = mModuleSaveData.GetInt("gridrows");
   mNumSteps = mModuleSaveData.GetInt("gridsteps");
   mShowStepControls = mModuleSaveData.GetBool("stepcontrols");
   mLength = MIN(mLength, mNumSteps);
   mGrid->SetGrid(mNumSteps, mNoteRange);
   mVelocityGrid->SetGrid(mNumSteps, 1);
   mLengthSlider->SetExtents(1, mNumSteps);
   mLoopResetPointSlider->SetExtents(0, mNumSteps);
   UpdateVelocityGridPos();
}

namespace
{
   const int kSaveStateRev = 1;
}

void NoteStepSequencer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mGrid->SaveState(out);
   mVelocityGrid->SaveState(out);
}

void NoteStepSequencer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mGrid->LoadState(in);
   mVelocityGrid->LoadState(in);
   GridUpdated(mGrid, 0, 0, 0, 0);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);
}
