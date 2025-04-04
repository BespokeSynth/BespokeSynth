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

    NoteExpressionRouter.h
    Created: 19 Dec 2019 10:40:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <array>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"
#include "exprtk.hpp"

class NoteExpressionRouter : public INoteReceiver, public INoteSource, public IDrawableModule, public ITextEntryListener
{
public:
   NoteExpressionRouter();
   static IDrawableModule* Create() { return new NoteExpressionRouter(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;

   void TextEntryComplete(TextEntry* entry) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   static const int kMaxDestinations = 5;
   AdditionalNoteCable* mDestinationCables[kMaxDestinations]{ nullptr };
   float mWidth{ 200 };
   float mHeight{ 20 };

private:
   exprtk::symbol_table<float> mSymbolTable;
   float mSTNote{ 0 }, mSTVelocity{ 0 }; // bound to the symbol table
   std::array<exprtk::expression<float>, kMaxDestinations> mExpressions;
   TextEntry* mExpressionWidget[kMaxDestinations]{ nullptr };
   char mExpressionText[kMaxDestinations][MAX_TEXTENTRY_LENGTH]{};
};
