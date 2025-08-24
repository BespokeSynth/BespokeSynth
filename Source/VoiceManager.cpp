/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  VoiceManager.cpp
//  Bespoke
//
//  Created by Noxy Nixie on 8/20/2025.
//
//

#include "VoiceManager.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

VoiceManager::VoiceManager()
{
}

void VoiceManager::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK(113);
   DROPDOWN(mVoiceDistributionModeSelector, "voice distribution mode", reinterpret_cast<int*>(&mVoiceDistributionMode), 80);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mVoiceLimitSlider, "voices", &mVoiceLimit, 1, 16);
   ENDUIBLOCK0();

   float w, h;
   GetModuleDimensions(w, h);

   for (int i = 0; i < kNumVoices; ++i)
   {
      auto additionalCable = new AdditionalNoteCable();
      auto patchCable = new PatchCableSource(this, kConnectionType_Note);
      patchCable->SetName(("Voice " + ofToString(i)).c_str());
      patchCable->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
      patchCable->SetManualPosition(w - 10, 30 + (i * 18));
      additionalCable->SetPatchCableSource(patchCable);
      AddPatchCableSource(additionalCable->GetPatchCableSource());
      mDestinationCables.push_back(additionalCable);

      mVoices[i].voiceIdx = i;
      mVoices[i].voiceActionSelector = new DropdownList(this, ("voice action " + std::to_string(i)).c_str(), 3, 22 + i * 18, reinterpret_cast<int*>(&mVoices[i].mVoiceAction));
      mVoices[i].voiceActionSelector->AddLabel("Unchanged", VoiceManagerVoice::Unchanged);
      mVoices[i].voiceActionSelector->AddLabel("Filter/Skip", VoiceManagerVoice::Filter);
      mVoices[i].voiceActionSelector->AddLabel("Remap", VoiceManagerVoice::Remap);
      mVoices[i].mVoiceDestination = i;
      mVoices[i].voiceDestinationSlider = new IntSlider(this, ("voice dest " + std::to_string(i)).c_str(), mVoices[i].voiceActionSelector, AnchorDirection::kAnchor_Right, 100, 15, &mVoices[i].mVoiceDestination, -1, kNumVoices - 1);
      mVoices[i].voiceDestinationSlider->SetShowing(false);
   }

   GetPatchCableSource()->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);

   mVoiceDistributionModeSelector->AddLabel("Rotate", Rotate); // Selects next unused voice on every note
   mVoiceDistributionModeSelector->AddLabel("Reuse", Reuse); // Selects next voice if a voice wasn't recently used for the given pitch else use that voice instead
   mVoiceDistributionModeSelector->AddLabel("Reset", Reset); // Always use the first unused voice
   mVoiceDistributionModeSelector->AddLabel("Random", Random); // Randomly select a voice that is unused
   mVoiceDistributionModeSelector->AddLabel("Ignore", Ignore); // Don't distribute voices
}

void VoiceManager::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (auto source : GetPatchCableSources())
      source->Draw();

   for (auto& mVoice : mVoices)
   {
      mVoice.voiceActionSelector->Draw();
      mVoice.voiceDestinationSlider->Draw();
   }

   mVoiceDistributionModeSelector->Draw();
   mVoiceLimitSlider->Draw();
}

void VoiceManager::UpdateVoiceUIVisibility()
{
   for (int i = 0; i < kNumVoices; ++i)
   {
      bool showVoice = mVoiceLimit > i;
      bool isReassign = mVoices[i].mVoiceAction == VoiceManagerVoice::Remap;
      bool isFiltered = mVoices[i].mVoiceAction == VoiceManagerVoice::Filter;
      bool showDestSlider = isReassign && showVoice;
      bool isDestination = false;
      for (auto& voice : mVoices)
      {
         if (voice.mVoiceAction == VoiceManagerVoice::Remap &&
             voice.mVoiceDestination == i &&
             voice.voiceIdx != voice.mVoiceDestination &&
             voice.voiceIdx < mVoiceLimit)
         {
            isDestination = true;
            break;
         }
      }
      bool showPatchCable = isDestination || (showVoice && !isFiltered && !(isReassign && mVoices[i].mVoiceDestination != i));

      mVoices[i].voiceActionSelector->SetShowing(showVoice);
      mVoices[i].voiceDestinationSlider->SetShowing(showDestSlider);
      GetPatchCableSource(i + 1)->SetShowing(showPatchCable);
   }
}

void VoiceManager::GetModuleDimensions(float& width, float& height)
{
   int highestvoice = -1;
   for (const auto& voice : mVoices)
   {
      if (voice.mVoiceAction == VoiceManagerVoice::Remap && voice.mVoiceDestination > highestvoice)
         highestvoice = voice.mVoiceDestination;
   }
   width = mWidth;
   height = mHeight + MAX(mVoiceLimit, highestvoice + 1) * 18;
}

void VoiceManager::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   UpdateVoiceUIVisibility();
}

void VoiceManager::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   UpdateVoiceUIVisibility();
}
void VoiceManager::PlayNote(NoteMessage note)
{
   if (note.velocity < 1) // Note off, redirect to the voice that had the same pitch
   {
      note.voiceIdx = mVoiceMapping[note.pitch];
      if (note.voiceIdx > -1)
      {
         mVoices[note.voiceIdx].lastNote = note;
         mVoices[note.voiceIdx].lastUsedTime = gTime;
         if (mVoices[note.voiceIdx].mVoiceAction == VoiceManagerVoice::Remap)
            note.voiceIdx = mVoices[note.voiceIdx].mVoiceDestination;
         if (note.voiceIdx > -1)
            mDestinationCables[note.voiceIdx]->PlayNoteOutput(note);
      }
      PlayNoteOutput(note);
      return;
   }
   if (note.voiceIdx == -1 && mVoiceDistributionMode != Ignore)
   {
      // Reuse
      auto found = false;
      if (mVoiceDistributionMode == Reuse)
      {
         // See if we have a previous voice that has the same pitch
         for (const auto& voice : mVoices)
         {
            if (voice.voiceIdx >= mVoiceLimit)
               break;
            if (voice.lastNote.pitch == note.pitch && voice.mVoiceAction != VoiceManagerVoice::Filter)
            {
               note.voiceIdx = voice.voiceIdx;
               found = true;
               break;
            }
         }
      }
      // Rotate
      if (!found || mVoiceDistributionMode == Rotate) // Reuse failed or rotate mode
      {
         found = false;
         auto newVoice = mLastOutputVoice;
         for (int i = 0; i < MAX(kNumVoices, mVoiceLimit - 1); ++i)
         {
            if (++newVoice > kNumVoices - 1 || newVoice > mVoiceLimit)
               newVoice = 0;
            if (mVoices[newVoice].lastNote.velocity < 1 && mVoices[newVoice].mVoiceAction != VoiceManagerVoice::Filter)
            {
               found = true;
               note.voiceIdx = newVoice;
               break;
            }
         }
         if (!found) // No unused voices found, override the oldest note
         {
            // Find the oldest note to override
            auto oldestTime = std::numeric_limits<double>::max();
            auto oldestId = 0;
            for (const auto& voice : mVoices)
            {
               if (voice.voiceIdx >= mVoiceLimit)
                  break;
               if (voice.lastUsedTime < oldestTime && voice.mVoiceAction != VoiceManagerVoice::Filter)
               {
                  oldestTime = voice.lastUsedTime;
                  oldestId = voice.voiceIdx;
               }
            }
            note.voiceIdx = oldestId;
         }
      }
      // Reset
      if (mVoiceDistributionMode == Reset)
      {
         found = false;
         for (const auto& voice : mVoices)
         {
            if (voice.lastNote.velocity < 1 && voice.mVoiceAction != VoiceManagerVoice::Filter)
            {
               note.voiceIdx = voice.voiceIdx;
               found = true;
               break;
            }
         }
         if (!found) // All notes are being played
         {
            note.voiceIdx = mLastOutputVoice + 1;
            if (note.voiceIdx >= mVoiceLimit)
               note.voiceIdx = 0;
         }
      }
      // Random
      if (mVoiceDistributionMode == Random)
      {
         std::vector<int> voiceOptions;
         for (auto& voice : mVoices)
            if (voice.lastNote.velocity < 1 && voice.mVoiceAction != VoiceManagerVoice::Filter && voice.voiceIdx < mVoiceLimit)
               voiceOptions.push_back(voice.voiceIdx);
         if (voiceOptions.empty())
         {
            note.voiceIdx = mLastOutputVoice + 1;
            if (note.voiceIdx >= mVoiceLimit)
               note.voiceIdx = 0;
         }
         else
         {
            note.voiceIdx = voiceOptions[ofRandom(voiceOptions.size())];
         }
      }
      // Store last id
      mLastOutputVoice = note.voiceIdx;
      mVoices[note.voiceIdx].lastNote = note;
      mVoices[note.voiceIdx].lastUsedTime = gTime;
   }
   bool isFiltered = false;
   if (note.voiceIdx > -1)
   {
      // Wrap
      if (note.voiceIdx >= mVoiceLimit)
         note.voiceIdx %= mVoiceLimit;
      // Filtering
      if (mVoices[note.voiceIdx].mVoiceAction == VoiceManagerVoice::Filter)
         isFiltered = true;
      // Remap
      mVoiceMapping[note.pitch] = note.voiceIdx; // Remember which voice played this pitch before remapping
      if (mVoices[note.voiceIdx].mVoiceAction == VoiceManagerVoice::Remap)
      {
         note.voiceIdx = mVoices[note.voiceIdx].mVoiceDestination;
         isFiltered = false;
      }
      // Output the modified notes
      if (!isFiltered && note.voiceIdx > -1)
         mDestinationCables[note.voiceIdx]->PlayNoteOutput(note);
   }
   if (!isFiltered)
      PlayNoteOutput(note);
}

void VoiceManager::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void VoiceManager::SetUpFromSaveData()
{
}
