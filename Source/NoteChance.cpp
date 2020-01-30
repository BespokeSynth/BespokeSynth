/*
  ==============================================================================

    NoteChance.cpp
    Created: 29 Jan 2020 9:17:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteChance.h"
#include "SynthGlobals.h"

NoteChance::NoteChance()
: mChance(1)
{
}

NoteChance::~NoteChance()
{
}

void NoteChance::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mChanceSlider = new FloatSlider(this, "chance", 3, 4, 100, 15, &mChance, 0, 1);
}

void NoteChance::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mChanceSlider->Draw();
}

void NoteChance::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (ofRandom(1) <= mChance || velocity == 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteChance::GetModuleDimensions(int& width, int& height)
{
   width = 116;
   height = 20;
}

void NoteChance::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteChance::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
