/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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
/*
  ==============================================================================

    NoteExpressionRouter.cpp
    Created: 15 Oct 2021
    Author:  Paul Walker @baconpaul

  ==============================================================================
*/

#include "NoteExpressionRouter.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

NoteExpressionRouter::NoteExpressionRouter()
{
   mSymbolTable.add_variable("p", mSTNote);
   mSymbolTable.add_variable("v", mSTVelocity);

   for (auto i = 0; i < kMaxDestinations; ++i)
   {
      mExpressions[i].register_symbol_table(mSymbolTable);
      auto p = exprtk::parser<float>();
      p.compile("1", mExpressions[i]);
   }
}

void NoteExpressionRouter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (int i = 0; i < kMaxDestinations; ++i)
   {
      TEXTENTRY(mExpressionWidget[i], ("expression" + ofToString(i)).c_str(), 30, mExpressionText[i]);
      mDestinationCables[i] = new AdditionalNoteCable();
      mDestinationCables[i]->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
      mDestinationCables[i]->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
      AddPatchCableSource(mDestinationCables[i]->GetPatchCableSource());
      ofRectangle rect = mExpressionWidget[i]->GetRect(true);
      mDestinationCables[i]->GetPatchCableSource()->SetManualPosition(rect.getMaxX() + 10, rect.y + rect.height / 2);
   }
   ENDUIBLOCK(mWidth, mHeight);
   mWidth += 20;

   GetPatchCableSource()->SetEnabled(false);
}

void NoteExpressionRouter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxDestinations; ++i)
      mExpressionWidget[i]->Draw();
}

void NoteExpressionRouter::PlayNote(NoteMessage note)
{
   mSTNote = note.pitch;
   mSTVelocity = note.velocity;
   for (auto i = 0; i < kMaxDestinations; ++i)
   {
      auto rt = mExpressions[i].value();
      if (rt != 0)
      {
         mDestinationCables[i]->PlayNoteOutput(note);
      }
   }
}

void NoteExpressionRouter::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteExpressionRouter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteExpressionRouter::SetUpFromSaveData()
{
}

void NoteExpressionRouter::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void NoteExpressionRouter::TextEntryComplete(TextEntry* entry)
{
   if (strlen(entry->GetText()) == 0)
      return;

   for (auto i = 0; i < kMaxDestinations; ++i)
   {
      if (entry == mExpressionWidget[i])
      {
         auto p = exprtk::parser<float>();
         if (!p.compile(entry->GetText(), mExpressions[i]))
         {
            ofLog() << "Error parsing expression '" << entry->GetText() << "' " << p.error();
         }
      }
   }
}
