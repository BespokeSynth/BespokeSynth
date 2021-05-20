/*
  ==============================================================================

    PitchRemap.cpp
    Created: 7 May 2021 10:17:55pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PitchRemap.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

PitchRemap::PitchRemap()
{
   for (size_t i=0; i<mRemaps.size(); ++i)
   {
      mRemaps[i].mFromPitch = (int)i;
      mRemaps[i].mToPitch = (int)i;
   }
}

void PitchRemap::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   for (size_t i=0; i<mRemaps.size(); ++i)
   {
      TEXTENTRY_NUM(mRemaps[i].mFromPitchEntry, ("from"+ofToString(i)).c_str(), 4, &mRemaps[i].mFromPitch, 0, 127); UIBLOCK_SHIFTX(62);
      TEXTENTRY_NUM(mRemaps[i].mToPitchEntry, ("to"+ofToString(i)).c_str(), 4, &mRemaps[i].mToPitch, 0, 127); UIBLOCK_NEWLINE();
   }
   ENDUIBLOCK(mWidth, mHeight);
}

void PitchRemap::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   for (size_t i=0; i<mRemaps.size(); ++i)
   {
      mRemaps[i].mFromPitchEntry->Draw();
      ofRectangle rect = mRemaps[i].mFromPitchEntry->GetRect(true);
      ofLine(rect.getMaxX() + 5, rect.getCenter().y, rect.getMaxX() + 20, rect.getCenter().y);
      ofLine(rect.getMaxX() + 15, rect.getCenter().y-4, rect.getMaxX() + 20, rect.getCenter().y);
      ofLine(rect.getMaxX() + 15, rect.getCenter().y+4, rect.getMaxX() + 20, rect.getCenter().y);
      mRemaps[i].mToPitchEntry->Draw();
   }
}

void PitchRemap::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void PitchRemap::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (pitch >= 0 && pitch < 128)
   {
      if (velocity > 0)
      {
         mInputNotes[pitch].mOn = true;
         mInputNotes[pitch].mVelocity = velocity;
         mInputNotes[pitch].mVoiceIdx = voiceIdx;
      }
      else
      {
         mInputNotes[pitch].mOn = false;
      }
   }
   
   bool remapped = false;
   for (size_t i=0; i<mRemaps.size(); ++i)
   {
      if (pitch == mRemaps[i].mFromPitch)
      {
         PlayNoteOutput(time, mRemaps[i].mToPitch, velocity, voiceIdx, modulation);
         remapped = true;
         break;
      }
   }
   
   if (!remapped)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void PitchRemap::TextEntryComplete(TextEntry* entry)
{
   //TODO(Ryan) make this handle mappings changing while notes are input
   mNoteOutput.Flush(gTime+gBufferSizeMs);
}

void PitchRemap::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PitchRemap::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

