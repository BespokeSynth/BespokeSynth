//
//  Arpeggiator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "Arpeggiator.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PolyphonyMgr.h"

#define ARP_REST -100
#define ARP_HOLD -101

Arpeggiator::Arpeggiator()
: mUseHeldNotes(true)
, mInterval(kInterval_16n)
, mArpIndex(-1)
, mRestartOnPress(true)
, mUserPitch(-1)
, mLastPitch(-1)
, mIntervalSelector(nullptr)
, mUseHeldNotesCheckbox(nullptr)
, mRestartOnPressCheckbox(nullptr)
, mArpEntry(nullptr)
, mEasyButton(nullptr)
, mRandomLength(8)
, mRandomRange(24)
, mRandomRest(false)
, mRandomHold(false)
, mRepeatIsHold(true)
, mRepeatIsHoldCheckbox(nullptr)
, mRandomLengthSlider(nullptr)
, mRandomRangeSlider(nullptr)
, mRandomRestCheckbox(nullptr)
, mRandomHoldCheckbox(nullptr)
, mArpStep(1)
, mArpPingPongDirection(1)
, mArpStepSlider(nullptr)
, mResetOnDownbeat(false)
, mResetOnDownbeatCheckbox(nullptr)
, mViewGrid(false)
, mGrid(nullptr)
, mViewGridCheckbox(nullptr)
, mUpbeats(false)
, mUpbeatsCheckbox(nullptr)
, mPlayOnlyScaleNotes(false)
, mPlayOnlyScaleNotesCheckbox(nullptr)
, mCurrentOctaveOffset(0)
, mOctaveRepeats(1)
, mOctaveRepeatsSlider(nullptr)
{
   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
   TheScale->AddListener(this);
   
   bzero(mArpString, MAX_TEXTENTRY_LENGTH);
}

void Arpeggiator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",4,30,(int*)(&mInterval));
   mUseHeldNotesCheckbox = new Checkbox(this,"use held", 4, 50, &mUseHeldNotes);
   mRestartOnPressCheckbox = new Checkbox(this,"restart", 5, 84, &mRestartOnPress);
   mArpEntry = new TextEntry(this,"arpentry",3,13,10,mArpString);
   mEasyButton = new ClickButton(this,"easy",80,30);
   mRepeatIsHoldCheckbox = new Checkbox(this,"!repeat",4,67,&mRepeatIsHold);
   mRandomLengthSlider = new IntSlider(this,"r len",130,30,100,15,&mRandomLength,3,16);
   mRandomRangeSlider = new IntSlider(this,"r range",130,48,100,15,&mRandomRange,4,48);
   mRandomRestCheckbox = new Checkbox(this,"r rests",130,66,&mRandomRest);
   mRandomHoldCheckbox = new Checkbox(this,"r holds",130,84,&mRandomHold);
   mArpStepSlider = new IntSlider(this,"step",140,2,60,12,&mArpStep,-3,3);
   mOctaveRepeatsSlider = new IntSlider(this,"octaves",mArpStepSlider,kAnchor_Below,60,12,&mOctaveRepeats,1,4);
   mResetOnDownbeatCheckbox = new Checkbox(this,"downbeat",5,100,&mResetOnDownbeat);
   mGrid = new UIGrid(5,120,230,100,8,24, this);
   mViewGridCheckbox = new Checkbox(this,"view grid",90,102,&mViewGrid);
   mUpbeatsCheckbox = new Checkbox(this,"upbeats",180,102,&mUpbeats);
   mPlayOnlyScaleNotesCheckbox = new Checkbox(this,"scale",90,60,&mPlayOnlyScaleNotes);
   
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
   
   mGrid->SetSingleColumnMode(true);
   mGrid->SetFlip(true);
}

Arpeggiator::~Arpeggiator()
{
   TheTransport->RemoveListener(this);
   TheScale->RemoveListener(this);
}

void Arpeggiator::Init()
{
   IDrawableModule::Init();
   
   GenerateRandomArpeggio();
}

void Arpeggiator::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mIntervalSelector->Draw();
   mUseHeldNotesCheckbox->Draw();
   mEasyButton->Draw();
   mRepeatIsHoldCheckbox->Draw();
   mRandomLengthSlider->Draw();
   mRandomRangeSlider->Draw();
   mRandomRestCheckbox->Draw();
   mRandomHoldCheckbox->Draw();
   mRestartOnPressCheckbox->Draw();
   mArpStepSlider->Draw();
   mOctaveRepeatsSlider->Draw();
   mResetOnDownbeatCheckbox->Draw();
   mUpbeatsCheckbox->Draw();
   mPlayOnlyScaleNotesCheckbox->Draw();
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mArpEntry)
   {
      mArpEntry->Draw();
   }
   else
   {
      ofSetColor(200,200,200,gModuleDrawAlpha);
      string chord;
      for (int i=0; i<mChord.size(); ++i)
         chord += GetArpNoteDisplay(mChord[i].pitch) + " ";
      DrawTextNormal(chord,5,25);
      ofSetColor(0,255,0,gModuleDrawAlpha);
      string pad;
      for (int i=0; i<mChord.size(); ++i)
      {
         if (i != mArpIndex)
         {
            pad += GetArpNoteDisplay(mChord[i].pitch) + " ";
         }
         else
         {
            float w = gFont.GetStringWidth(pad, 15, K(isRenderThread));
            DrawTextNormal(GetArpNoteDisplay(mChord[i].pitch),5+w+pad.length()/5.0f,25);
            break;
         }
      }
   }

   mViewGridCheckbox->Draw();
   if (mViewGrid)
   {
      mGrid->Draw();
      
      ofPushStyle();
      ofFill();
      for (int i=0;i<mGrid->GetRows();++i)
      {
         if (i%TheScale->GetTet() == 0)
            ofSetColor(0,255,0,80);
         else if ((i+7)%TheScale->GetTet() == 0)
            ofSetColor(200,150,0,80);
         else
            continue;
         
         float gridX,gridY;
         mGrid->GetPosition(gridX, gridY, true);
         float boxHeight = (float(mGrid->GetHeight())/mGrid->GetRows());
         float y = gridY + mGrid->GetHeight() - i*boxHeight;
         ofRect(gridX,y-boxHeight,mGrid->GetWidth(),boxHeight);
      }
      ofPopStyle();
   }
}

void Arpeggiator::OnScaleChanged()
{
   mChordMutex.lock();
   mChord.clear();
   mChordMutex.unlock();
}

string Arpeggiator::GetArpNoteDisplay(int note)
{
   if (note == ARP_REST)
      return "x";
   else if (note == ARP_HOLD)
      return "h";
   else if (mUseHeldNotes && mPlayOnlyScaleNotes && note != TheScale->MakeDiatonic(note))
      return "x";
   else
      return ofToString(note);
}

void Arpeggiator::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (mArpEntry->TestClick(x,y,right))
   {
      string arpString;
      for (int i=0; i<mChord.size(); ++i)
      {
         if (mChord[i].pitch == ARP_REST)
            arpString += "x";
         else if (mChord[i].pitch == ARP_HOLD)
            arpString += "h";
         else
            arpString += ofToString(mChord[i].pitch);
         if (i < mChord.size() - 1)
            arpString += " ";
      }
      StringCopy(mArpString, arpString.c_str(), 128);
   }

   mGrid->TestClick(x,y,right);
}

void Arpeggiator::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool Arpeggiator::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   return false;
}

void Arpeggiator::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
   if (checkbox == mUseHeldNotesCheckbox)
   {
      if (mUseHeldNotes)
      {
         mChordMutex.lock();
         mChord.clear();
         mChordMutex.unlock();
      }
      else
      {
         GenerateRandomArpeggio();
         mUserPitch = -1;
      }
   }
   if (checkbox == mUpbeatsCheckbox)
   {
      UpdateInterval();
   }
}

void Arpeggiator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      if (!mUseHeldNotes)
      {
         if (mRestartOnPress)
            mArpIndex = -1;
      
         mUserPitch = pitch;
      }
      else
      {
         mUserPitch = 0;
         mChordMutex.lock();
         mChord.push_back(ArpNote(pitch,velocity, voiceIdx, modulation));
         mChordMutex.unlock();
      }
   }
   else
   {
      if (!mUseHeldNotes)
      {
         if (pitch == mUserPitch)
            mUserPitch = -1;
      }
      else
      {
         mChordMutex.lock();
         for (auto iter = mChord.begin(); iter != mChord.end(); ++iter)
         {
            if (iter->pitch == pitch)
            {
               mChord.erase(iter);
               break;
            }
         }
         mChordMutex.unlock();
      }
   } 
}

void Arpeggiator::OnTimeEvent(double time)
{
   if (!mEnabled)
      return;

   if (mViewGrid)
   {
      mChordMutex.lock();
      mChord.clear();
      for (int i=0; i<mRandomLength; ++i)
      {
         int val = ARP_REST;
         for (int j=0; j<mRandomRange+1; ++j)
         {
            if (mGrid->GetValRefactor(j,i))
            {
               val = j;
               break;
            }
         }
         mChord.push_back(ArpNote(val,80,-1,ModulationParameters()));
      }
      mChordMutex.unlock();
   }

   if (mChord.size() == 0)
   {
      if (mLastPitch != -1)
         PlayNoteOutput(time, mLastPitch, 0, -1);
      mLastPitch = -1;
      return;
   }

   if (mArpStep != 0)
   {
      mArpIndex += mArpStep;
      if (mChord.size() > 0)
      {
         while (mArpIndex >= (int)mChord.size())
         {
            mArpIndex -= mChord.size();
            mCurrentOctaveOffset = (mCurrentOctaveOffset + 1) % mOctaveRepeats;
         }
         while (mArpIndex <0)
            mArpIndex += mChord.size();
      }
   }
   else //pingpong
   {
      assert (mArpPingPongDirection == 1 || mArpPingPongDirection == -1);
      mArpIndex += mArpPingPongDirection;
      if (mChord.size() >= 2)
      {
         if (mArpIndex < 0)
         {
            mArpIndex = 1;
            mArpPingPongDirection = 1;
         }
         if (mArpIndex > mChord.size() - 1)
         {
            mArpIndex = (int)mChord.size() - 2;
            mArpPingPongDirection = -1;
         }
      }
      else
      {
         mArpIndex = ofClamp(mArpIndex,0,mChord.size()-1);
      }
   }

   if (mResetOnDownbeat && TheTransport->GetQuantized(time, mInterval) == 0)
      mArpIndex = 0;

   int offPitch = -1;
   if (mLastPitch >= 0 && mChord[mArpIndex].pitch != ARP_HOLD)
   {
      offPitch = mLastPitch;
   }
   if (mUserPitch >= 0 && mChord.size())
   {
      ArpNote current = mChord[mArpIndex];
      if (current.pitch == ARP_REST)
      {
         mLastPitch = -1;
      }
      else if (current.pitch == ARP_HOLD)
      {
         //nothing
      }
      else
      {
         int nonDiatonicPitch = mUserPitch + current.pitch;
         int outPitch = TheScale->MakeDiatonic(nonDiatonicPitch);
         
         if (mUseHeldNotes && !mPlayOnlyScaleNotes)
            outPitch = nonDiatonicPitch;
         
         outPitch += mCurrentOctaveOffset * TheScale->GetTet();

         if (mLastPitch == outPitch && mRepeatIsHold)
         {
            //if it's the same note and repeats are treated as holds, clear note off
            offPitch = -1;
         }
         else if (mUseHeldNotes && mPlayOnlyScaleNotes &&
                  outPitch != nonDiatonicPitch) //pitch wasn't in scale and it was a user-pressed one
         {
            mLastPitch = -1; //treat non-diatonic notes as a rest in this mode
         }
         else
         {
            if (mLastPitch == outPitch)   //same note, play noteoff first
            {
               PlayNoteOutput(time, mLastPitch, 0, -1);
               offPitch = -1;
            }
            float pressure = current.modulation.pressure ? current.modulation.pressure->GetValue(0) : 0;
            PlayNoteOutput(time, outPitch, ofClamp(current.vel+127*pressure,0,127), current.voiceIdx, current.modulation);
            mLastPitch = outPitch;
         }
      }
   }
   if (offPitch != -1)
   {
      PlayNoteOutput(time, offPitch, 0, -1);
      if (offPitch == mLastPitch)
         mLastPitch = -1;
   }

   mGrid->SetHighlightCol(mArpIndex);
}

void Arpeggiator::TextEntryComplete(TextEntry* entry)
{
   mChordMutex.lock();
   mChord.clear();
   vector<string> tokens = ofSplitString(mArpString," ");
   for (int i=0; i<tokens.size(); ++i)
   {
      if (tokens[i] == "x")
         mChord.push_back(ArpNote(ARP_REST,0,-1,ModulationParameters()));
      else if (tokens[i] == "h")
         mChord.push_back(ArpNote(ARP_HOLD,0,-1,ModulationParameters()));
      else
         mChord.push_back(ArpNote(atoi(tokens[i].c_str()),80,-1,ModulationParameters()));
   }
   mChordMutex.unlock();
   SyncGridToArp();
}

void Arpeggiator::GenerateRandomArpeggio()
{
   if (mUseHeldNotes)
      return;
   
   mChordMutex.lock();
   mChord.clear();
   mChord.push_back(ArpNote(12,80,-1,ModulationParameters()));
   for (int i=1; i<mRandomLength; ++i)
   {
      int choose = rand() % 5;
      if (choose == 0 && mRandomRest)
         mChord.push_back(ArpNote(ARP_REST,0,-1,ModulationParameters()));
      else if (choose == 1 && mRandomHold)
         mChord.push_back(ArpNote(ARP_HOLD,0,-1,ModulationParameters()));
      else
         mChord.push_back(ArpNote(rand()%mRandomRange,rand()%100+27,-1,ModulationParameters()));
   }
   mChordMutex.unlock();
   SyncGridToArp();
}

void Arpeggiator::UpdateInterval()
{
   float upbeatLength = 0;
   if (mUpbeats)
      upbeatLength = (1.0f/TheTransport->CountInStandardMeasure(mInterval)) / 2.0f;
   
   TheTransport->UpdateListener(this, mInterval, OffsetInfo(upbeatLength, false));
}

void Arpeggiator::ButtonClicked(ClickButton* button)
{
   if (button == mEasyButton)
      GenerateRandomArpeggio();
}

void Arpeggiator::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      UpdateInterval();
}

void Arpeggiator::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mArpStepSlider)
   {
      if (oldVal > 0)
         mArpPingPongDirection = 1;
      else if (oldVal < 0)
         mArpPingPongDirection = -1;
   }
   if (slider == mRandomLengthSlider ||
       slider == mRandomRangeSlider)
   {
      mGrid->SetGrid(mRandomLength, mRandomRange+1);
      GenerateRandomArpeggio();
   }
}

void Arpeggiator::SyncGridToArp()
{
   mGrid->Clear();
   int max = mRandomRange;
   for (int i=0; i<mChord.size(); ++i)
   {
      if (mChord[i].pitch > max)
         max = mChord[i].pitch;

      if (mChord[i].pitch < 0)
         continue;
      
      if (mChord[i].pitch != ARP_REST && mChord[i].pitch != ARP_HOLD)
         mGrid->SetValRefactor(mChord[i].pitch,i,1);
   }
   mRandomLength = (int)mChord.size();
   mRandomRange = max;
   mGrid->SetGrid(mRandomLength, mRandomRange+1);
}

void Arpeggiator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Arpeggiator::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

