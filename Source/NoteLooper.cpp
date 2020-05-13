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
: mPlay(false)
, mPlayCheckbox(nullptr)
, mRecord(false)
, mRecordCheckbox(nullptr)
, mNumBars(1)
, mNumBarsSlider(nullptr)
, mOctave(0)
, mOctaveSlider(nullptr)
, mClearButton(nullptr)
, mOverdub(false)
, mOverdubCheckbox(nullptr)
, mNumHeldNotes(0)
, mNumBarsDecrement(nullptr)
, mNumBarsIncrement(nullptr)
{
   TheTransport->AddAudioPoller(this);
   
   for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
      mNoteroll[i].mValid = false;
   for (int i=0; i<NOTELOOPER_MAX_CHORD; ++i)
      mCurrentNotes[i].mPitch = -1;
   for (int i=0; i<NOTELOOPER_NOTE_RANGE; ++i)
      mHeldNotes[i] = false;
}

void NoteLooper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPlayCheckbox = new Checkbox(this,"play",4,24,&mPlay);
   mRecordCheckbox = new Checkbox(this,"rec",45,24,&mRecord);
   mNumBarsSlider = new IntSlider(this,"num bars",113,22,85,15,&mNumBars,1,8);
   mOctaveSlider = new IntSlider(this,"octave",230,5,70,15,&mOctave,-3,3);
   mClearButton = new ClickButton(this,"clear",215,22);
   mOverdubCheckbox = new Checkbox(this,"overdub",100,4,&mOverdub);
   mNumBarsDecrement = new ClickButton(this,"<",100,22);
   mNumBarsIncrement = new ClickButton(this,">",199,22);
}

NoteLooper::~NoteLooper()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteLooper::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   mPlayCheckbox->Draw();
   mRecordCheckbox->Draw();
   mNumBarsSlider->Draw();
   mNumBarsDecrement->Draw();
   mNumBarsIncrement->Draw();
   mClearButton->Draw();
   mOctaveSlider->Draw();
   mOverdubCheckbox->Draw();
   
   DrawTextNormal(ofToString(mNumHeldNotes), 170, 13);
   
   int maxPitch = -1;
   int minPitch = 128;
   for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
   {
      if (mNoteroll[i].mValid)
      {
         if (mNoteroll[i].mPitch > maxPitch)
            maxPitch = mNoteroll[i].mPitch;
         if (mNoteroll[i].mPitch < minPitch)
            minPitch = mNoteroll[i].mPitch;
      }
   }
   
   ofPushStyle();
   ofFill();
   float starty = 40;
   float height = 90;
   float startx = 10;
   float width = 290;
   float pos = TheTransport->GetMeasurePos(gTime);
   pos += TheTransport->GetMeasure(gTime) % mNumBars;
   if (maxPitch != -1)
   {
      float numTones = maxPitch-minPitch + 1;
      for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
      {
         if (mNoteroll[i].mValid)
         {
            float x = startx+mNoteroll[i].mPos/mNumBars*width;
            float y = starty+(1-1/numTones-((mNoteroll[i].mPitch-minPitch)/numTones))*height;
            
            if (mNoteroll[i].mVelocity > 0)
            {
               int noteOff = mNoteroll[i].mAssociatedEvent;
               float color = mNoteroll[i].mVelocity * 2;
               ofSetColor(color,color,color,gModuleDrawAlpha);
               
               float endx;
               if (noteOff != -1)
                  endx = startx+mNoteroll[noteOff].mPos/mNumBars*width;
               else
                  endx = startx+pos/mNumBars*width;
               
               if (x <= endx)
               {
                  ofRect(x,y,endx-x,height/numTones);
               }
               else
               {
                  ofRect(x,y,width-x,height/numTones);
                  ofRect(startx,y,endx-startx,height/numTones);
               }
               ofSetColor(255,0,0);
               if (noteOff == -1)
                  DrawTextNormal("x",x,y+8);
            }
            else
            {
               ofSetColor(0,0,0,gModuleDrawAlpha);
               ofRect(x,y,1,height/numTones);
            }
         }
      }
   }
      
   ofSetColor(0,255,0,gModuleDrawAlpha);
   float x = startx+pos/mNumBars*width;
   ofRect(x,starty,1,height);
   
   ofPopStyle();
}

void NoteLooper::OnTransportAdvanced(float amount)
{
   PROFILER(NoteLooper);
   
   float pos = TheTransport->GetMeasurePos(gTime);
   pos += TheTransport->GetMeasure(gTime) % mNumBars;
   float lastPos = pos-amount;
   
   for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
   {
      NoteEvent& note = mNoteroll[i];
      if (note.mValid)
      {
         if (note.mPos <= pos && note.mPos > lastPos)
         {
            if (!note.mJustPlaced)
            {
               if (mOverdub && mNumHeldNotes > 0)
               {
                  note.mValid = false;
                  if (note.mAssociatedEvent != -1)
                  {
                     mNoteroll[note.mAssociatedEvent].mValid = false;   //disable associated note on/off as well
                     TriggerRecordedNote(note.mPitch, 0);
                  }
               }
               else
               {
                  TriggerRecordedNote(note.mPitch, note.mVelocity);
               }
            }
         }
      }
   }
   
   for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
   {
      NoteEvent& note = mNoteroll[i];
      if (note.mValid && note.mJustPlaced)
         note.mJustPlaced = false;
   }
}

void NoteLooper::TriggerRecordedNote(int pitch, int velocity)
{
   pitch += mOctave*12;
   if (mPlay)
      //PlayNoteOutput fix
      PlayNoteOutput(gTime, pitch, velocity, -1);
   
   for (int j=0; j<NOTELOOPER_MAX_CHORD; ++j)
   {
      if (velocity == 0)
      {
         if (mCurrentNotes[j].mPitch == pitch)
         {
            //remove my tone
            mCurrentNotes[j].mPitch = -1;
         }
      }
      else
      {
         if (mCurrentNotes[j].mPitch == -1)
         {
            //add my tone
            mCurrentNotes[j].mPitch = pitch;
            mCurrentNotes[j].mVelocity = velocity;
            break;
         }
      }
   }
}

void NoteLooper::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mRecord)
   {
      RecordNote(time, pitch, velocity);
      
      if (pitch >= 0 && pitch < NOTELOOPER_NOTE_RANGE)
      {
         if (velocity)
            mHeldNotes[pitch] = true;
         else
            mHeldNotes[pitch] = false;
      }
      
      int numHeldNotes = 0;
      for (int i=0; i<NOTELOOPER_NOTE_RANGE; ++i)
         numHeldNotes += mHeldNotes[i] ? 1 : 0;
      mNumHeldNotes = numHeldNotes;
   }
   
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteLooper::RecordNote(double time, int pitch, int velocity)
{
   float pos = TheTransport->GetMeasurePos(time);
   pos += TheTransport->GetMeasure(time) % mNumBars;
   
   for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
   {
      NoteEvent& note = mNoteroll[i];
      if (!note.mValid) // find first available note slot
      {
         note.mValid = true;
         note.mPos = pos;
         note.mPitch = pitch;
         note.mVelocity = velocity;
         note.mJustPlaced = true;
         note.mAssociatedEvent = -1;
         if (velocity == 0)
         {
            float closestPos = mNumBars;
            int noteOnIdx = -1;
            
            for (int j=0; j<NOTELOOPER_MAX_NOTES; ++j)
            {
               if (mNoteroll[j].mValid &&
                   mNoteroll[j].mPitch == pitch &&
                   mNoteroll[j].mVelocity > 0 &&
                   mNoteroll[j].mAssociatedEvent == -1)
               {
                  float distance;
                  if (pos >= mNoteroll[j].mPos)
                     distance = pos - mNoteroll[j].mPos;
                  else
                     distance = pos - (mNoteroll[j].mPos - 1);
                  
                  if (distance < closestPos)
                  {
                     closestPos = distance;
                     noteOnIdx = j;
                  }
               }
            }
            
            if (noteOnIdx != -1)
            {
               note.mAssociatedEvent = noteOnIdx;  //associate our note off with the note on
               mNoteroll[noteOnIdx].mAssociatedEvent = i;   //associate note on with this note off
            }
         }
         break;
      }
   }
}

void NoteLooper::Clear()
{
   for (int i=0; i<NOTELOOPER_MAX_NOTES; ++i)
   {
      mNoteroll[i].mValid = false;
   }
   mNoteOutput.Flush(gTime);
}

void NoteLooper::StopRecording()
{
   mRecord = false;
   for (int i=0; i<NOTELOOPER_NOTE_RANGE; ++i)
   {
      if (mHeldNotes[i])
      {
         mHeldNotes[i] = false;
         RecordNote(gTime, i, 0);
      }
   }
}

void NoteLooper::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mPlayCheckbox)
   {
      if (mPlay == false)
      {
         mNoteOutput.Flush(gTime);
         StopRecording();
      }
   }
   if (checkbox == mRecordCheckbox)
   {
      if (mRecord)
      {
         if (!mPlay)
         {
            mPlay = true;
            Clear(); //start anew
         }
         //otherwise, just keep playing and allow us to add to the recording
      }
      else
      {
         StopRecording();
      }
   }
}

void NoteLooper::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteLooper::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mOctaveSlider)
   {
      if (mPlay)
         Retrigger();
   }
}

void NoteLooper::Retrigger()
{
   mNoteOutput.Flush(gTime);
   //retrigger in new octave
   for (int i=0; i<NOTELOOPER_MAX_CHORD; ++i)
   {
      if (mCurrentNotes[i].mPitch >= 0)
      {
         int pitch = mCurrentNotes[i].mPitch;
         pitch += mOctave*12;
         //PlayNoteOutput fix
         PlayNoteOutput(gTime, pitch, mCurrentNotes[i].mVelocity, -1);
      }
   }
}

void NoteLooper::ButtonClicked(ClickButton* button)
{
   int minBars, maxBars;
   mNumBarsSlider->GetRange(minBars, maxBars);
   if (button == mClearButton)
      Clear();
   if (button == mNumBarsDecrement && mNumBars > minBars)
      --mNumBars;
   if (button == mNumBarsIncrement && mNumBars < maxBars)
      ++mNumBars;
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


