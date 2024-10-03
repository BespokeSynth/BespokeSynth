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
//
//  LabelDisplay.h
//  Bespoke
//
//  Created by Noxy Nixie on 03/19/2024.
//
//

#pragma once

#include <utility>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "TextEntry.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "UIControlMacros.h"

class LabelDisplay : public IDrawableModule, public ITextEntryListener, public IIntSliderListener, public IDropdownListener
{
public:
   LabelDisplay();
   virtual ~LabelDisplay();
   static IDrawableModule* Create() { return new LabelDisplay(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void TextEntryComplete(TextEntry* entry) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   float mWidth{ 200 };
   float mHeight{ 20 };

   Checkbox* mShowControlsCheckbox{ nullptr };
   bool mShowControls{ true };

   char mLabel[MAX_TEXTENTRY_LENGTH]{ "Label" };
   TextEntry* mLabelEntry{ nullptr };
   int mLabelSize{ 40 };
   IntSlider* mLabelSizeSlider{ nullptr };

   RetinaTrueTypeFont mLabelFont{ gFont };
   int mLabelFontIndex{ 0 };
   DropdownList* mLabelFontDropdown{ nullptr };

   struct LabelFont
   {
      LabelFont(std::string _name, RetinaTrueTypeFont _font)
      {
         name = std::move(_name);
         font = std::move(_font);
      }
      std::string name{ "Normal" };
      RetinaTrueTypeFont font{ gFont };
   };
   std::vector<LabelFont> mFonts{};

   ofColor mLabelColor{ ofColor::white };
   int mLabelColorIndex{ 0 };
   DropdownList* mLabelColorDropdown{ nullptr };

   struct LabelColor
   {
      LabelColor(std::string _name, const ofColor _color)
      {
         name = std::move(_name);
         color = _color;
      }
      std::string name{ "White" };
      ofColor color{ ofColor::white };
   };
   std::vector<LabelColor> mColors{};
};
