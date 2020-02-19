/*
  ==============================================================================

    NoteStrummer.cpp
    Created: 2 Apr 2018 9:27:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteStrummer.h"
#include "SynthGlobals.h"
#include "Scale.h"

NoteStrummer::NoteStrummer()
: mStrum(0)
, mLastStrumPos(0)
, mStrumSlider(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

NoteStrummer::~NoteStrummer()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteStrummer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mStrumSlider = new FloatSlider(this, "strum", 4, 4, 192, 15, &mStrum, 0, 1);
}

void NoteStrummer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mStrumSlider->Draw();
   
   int numNotes = (int)mNotes.size();
   int i=0;
   for (auto pitch : mNotes)
   {
      float pos = float(i + .5f) / numNotes;
      DrawTextNormal(NoteName(pitch), mStrumSlider->GetPosition(true).x + pos * mStrumSlider->IClickable::GetDimensions().x, mStrumSlider->GetPosition(true).y + mStrumSlider->IClickable::GetDimensions().y + 12);
      ++i;
   }
}

void NoteStrummer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mNotes.push_back(pitch);
   }
   else
   {
      PlayNoteOutput(time, pitch, 0, voiceIdx, modulation);
      mNotes.remove(pitch);
   }
}

void NoteStrummer::OnTransportAdvanced(float amount)
{
   int numNotes = (int)mNotes.size();
   
   for (int i=0; i<gBufferSize; ++i)
   {
      ComputeSliders(i);
      
      int index=0;
      for (auto pitch : mNotes)
      {
         float pos = float(index + .5f) / numNotes;
         float change = mStrum - mLastStrumPos;
         float offset = pos - mLastStrumPos;
         bool wraparound = fabsf(change) > .99f;
         if (change * offset > 0 && //same direction
             fabsf(offset) <= fabsf(change) &&
             !wraparound)
            PlayNoteOutput(gTime + i * gInvSampleRateMs, pitch, 127);
         ++index;
      }
      mLastStrumPos = mStrum;
   }
}

void NoteStrummer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteStrummer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteStrummer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
