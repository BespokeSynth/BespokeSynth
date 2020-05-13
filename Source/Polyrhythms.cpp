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
: mNumLines(4)
, mWidth(350)
, mHeight(mNumLines * 17 + 26)
{
   TheTransport->AddAudioPoller(this);
}

void Polyrhythms::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   for (int i=0; i<mNumLines; ++i)
   {
      mRhythmLines.push_back(new RhythmLine(this,i));
      mRhythmLines[i]->CreateUIControls();
   }
}

Polyrhythms::~Polyrhythms()
{
   TheTransport->RemoveAudioPoller(this);
   
   for (int i=0; i<mRhythmLines.size(); ++i)
      delete mRhythmLines[i];
}

void Polyrhythms::OnTransportAdvanced(float amount)
{
   PROFILER(Polyrhythms);
   
   if (!mEnabled)
      return;

   for (int i=0; i<mRhythmLines.size(); ++i)
   {
      int beats = mRhythmLines[i]->mGrid->GetCols();
      int oldQuantized;
      if (amount > TheTransport->GetMeasurePos(gTime))
         oldQuantized = -1;
      else
         oldQuantized = int((TheTransport->GetMeasurePos(gTime)-amount) * beats);
      int quantized = int(TheTransport->GetMeasurePos(gTime) * beats);

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

void Polyrhythms::Resize(float w, float h)
{
   mWidth = MAX(150,w);
   mHeight = mRhythmLines.size() * 17 + 26;
   for (int i=0; i<mRhythmLines.size(); ++i)
      mRhythmLines[i]->OnResize();
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
         mRhythmLines[i]->UpdateGrid();
   }
}

void Polyrhythms::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("lines", moduleInfo, 4, 1, 16, K(isTextField));

   SetUpFromSaveData();
}

void Polyrhythms::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mNumLines = mModuleSaveData.GetInt("lines");
}


RhythmLine::RhythmLine(Polyrhythms* owner, int index)
: mIndex(index)
, mGrid(nullptr)
, mLength(4)
, mLengthSelector(nullptr)
, mNote(index)
, mNoteSelector(nullptr)
, mOwner(owner)
{
}

void RhythmLine::CreateUIControls()
{
   mGrid = new UIGrid(4,4+mIndex*17,100,15,4,1, mOwner);
   mLengthSelector = new DropdownList(mOwner,("length"+ofToString(mIndex)).c_str(),-1,-1,&mLength);
   mNoteSelector = new DropdownList(mOwner,("note"+ofToString(mIndex)).c_str(),-1,-1,&mNote);
   
   mLengthSelector->AddLabel("3", 3);
   mLengthSelector->AddLabel("4", 4);
   mLengthSelector->AddLabel("5", 5);
   mLengthSelector->AddLabel("6", 6);
   mLengthSelector->AddLabel("7", 7);
   mLengthSelector->AddLabel("8", 8);
   mLengthSelector->AddLabel("9", 9);
   mLengthSelector->AddLabel("3x4", 12);
   mLengthSelector->AddLabel("4x4", 16);
   mLengthSelector->AddLabel("5x4", 20);
   mLengthSelector->AddLabel("6x4", 24);
   mLengthSelector->AddLabel("7x4", 28);
   mLengthSelector->AddLabel("8x4", 32);
   mLengthSelector->AddLabel("9x4", 36);
   
   for (int i=0; i<NUM_DRUM_HITS; ++i)
      mNoteSelector->AddLabel(DrumPlayer::GetDrumHitName(i).c_str(), i);
   
   OnResize();
}

void RhythmLine::OnResize()
{
   mGrid->SetDimensions(mOwner->IClickable::GetDimensions().x - 100, 15);
   mLengthSelector->PositionTo(mGrid, kAnchor_Right);
   mNoteSelector->PositionTo(mLengthSelector, kAnchor_Right);
}

void RhythmLine::UpdateGrid()
{
   mGrid->SetGrid(mLength,1);
   if (mLength % 4 == 0)
      mGrid->SetMajorColSize(mLength/4);
   else
      mGrid->SetMajorColSize(-1);
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

