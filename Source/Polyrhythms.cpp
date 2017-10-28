//
//  Polyrhythms.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/12/13.
//
//

#include "Polyrhythms.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "DrumPlayer.h"

Polyrhythms::Polyrhythms()
{
   TheTransport->AddAudioPoller(this);

   for (int i=0; i<8; ++i)
      mRhythmLines.push_back(new RhythmLine(this,10,40+i*15));
}

void Polyrhythms::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   for (int i=0; i<mRhythmLines.size(); ++i)
      mRhythmLines[i]->CreateUIControls();
}

Polyrhythms::~Polyrhythms()
{
   TheTransport->RemoveAudioPoller(this);
   
   for (int i=0; i<mRhythmLines.size(); ++i)
      delete mRhythmLines[i];
}

void Polyrhythms::OnTransportAdvanced(float amount)
{
   Profiler profiler("Polyrhythms");
   
   if (!mEnabled)
      return;

   for (int i=0; i<mRhythmLines.size(); ++i)
   {
      int beats = mRhythmLines[i]->mGrid->GetCols();
      int oldQuantized;
      if (amount > TheTransport->GetMeasurePos())
         oldQuantized = -1;
      else
         oldQuantized = int((TheTransport->GetMeasurePos()-amount) * beats);
      int quantized = int(TheTransport->GetMeasurePos() * beats);

      if (quantized != oldQuantized && mRhythmLines[i]->mGrid->GetValRefactor(0,quantized) > 0)
      {
         PlayNoteOutput(gTime, mRhythmLines[i]->mNote, 127, -1);
      }

      mRhythmLines[i]->mGrid->SetHighlightCol(quantized);
   }
}

void Polyrhythms::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   for (int i=0; i<mRhythmLines.size(); ++i)
      mRhythmLines[i]->Draw();
}

void Polyrhythms::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   for (int i=0; i<mRhythmLines.size(); ++i)
      mRhythmLines[i]->OnClicked(x,y,right);
}

void Polyrhythms::MouseReleased()
{
   IDrawableModule::MouseReleased();
   for (int i=0; i<mRhythmLines.size(); ++i)
      mRhythmLines[i]->MouseReleased();
}

bool Polyrhythms::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   for (int i=0; i<mRhythmLines.size(); ++i)
      mRhythmLines[i]->MouseMoved(x,y);
   return false;
}

void Polyrhythms::CheckboxUpdated(Checkbox* checkbox)
{
}

void Polyrhythms::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void Polyrhythms::DropdownUpdated(DropdownList* list, int oldVal)
{
   for (int i=0; i<mRhythmLines.size(); ++i)
   {
      if (list == mRhythmLines[i]->mLengthSelector)
         mRhythmLines[i]->mGrid->SetGrid(mRhythmLines[i]->mLength,1);
   }
}

void Polyrhythms::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Polyrhythms::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


RhythmLine::RhythmLine(Polyrhythms* owner, int x, int y)
: mGrid(nullptr)
, mLength(4)
, mLengthSelector(nullptr)
, mNote(0)
, mNoteSelector(nullptr)
, mOwner(owner)
, mPos(x,y)
{
}

void RhythmLine::CreateUIControls()
{
   mGrid = new Grid(mPos.x,mPos.y,180,15,4,1);
   mLengthSelector = new DropdownList(mOwner,"length",mPos.x+190,mPos.y,&mLength);
   mNoteSelector = new DropdownList(mOwner,"note",mPos.x+250,mPos.y,&mNote);
   
   mLengthSelector->AddLabel("3  ", 3);
   mLengthSelector->AddLabel("4", 4);
   mLengthSelector->AddLabel("5", 5);
   mLengthSelector->AddLabel("6", 6);
   mLengthSelector->AddLabel("7", 7);
   mLengthSelector->AddLabel("8", 8);
   mLengthSelector->AddLabel("9", 9);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mNoteSelector->AddLabel(DrumPlayer::GetDrumHitName(i).c_str(), i);
}

void RhythmLine::Draw()
{
   mGrid->Draw();
   mLengthSelector->Draw();
   mNoteSelector->Draw();
}

void RhythmLine::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   mGrid->TestClick(x, y, right);
}

void RhythmLine::MouseReleased()
{
   mGrid->MouseReleased();
}

void RhythmLine::MouseMoved(float x, float y)
{
   mGrid->NotifyMouseMoved(x, y);
}

