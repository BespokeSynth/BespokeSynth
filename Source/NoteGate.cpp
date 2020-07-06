//
//  NoteGate.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/22/16.
//
//

#include "NoteGate.h"
#include "SynthGlobals.h"

NoteGate::NoteGate()
: mGate(true)
{
}

NoteGate::~NoteGate()
{
}

void NoteGate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mGateCheckbox = new Checkbox(this, "gate", 3, 4, &mGate);
}

void NoteGate::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGateCheckbox->Draw();
}

void NoteGate::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mGate || velocity == 0 || !mEnabled)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteGate::GetModuleDimensions(float& width, float& height)
{
   width = 80;
   height = 20;
}

void NoteGate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteGate::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

