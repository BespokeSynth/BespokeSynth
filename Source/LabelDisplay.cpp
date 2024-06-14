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
//  LabelDisplay.cpp
//  Bespoke
//
//  Created by Noxy Nixie on 03/19/2024.
//
//

#include "LabelDisplay.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

#include "Slider.h"
#include "UIControlMacros.h"

LabelDisplay::LabelDisplay()
{
   mFonts.emplace_back("Normal", gFont);
   mFonts.emplace_back("Bold", gFontBold);
   mFonts.emplace_back("Fixed width", gFontFixedWidth);

   mColors.emplace_back("White", ofColor::white);
   mColors.emplace_back("Black", ofColor::black);
   mColors.emplace_back("Grey", ofColor::grey);
   mColors.emplace_back("Red", ofColor::red);
   mColors.emplace_back("Green", ofColor::green);
   mColors.emplace_back("Yellow", ofColor::yellow);
   mColors.emplace_back("Orange", ofColor::orange);
   mColors.emplace_back("Blue", ofColor::blue);
   mColors.emplace_back("Purple", ofColor::purple);
   mColors.emplace_back("Lime", ofColor::lime);
   mColors.emplace_back("Magenta", ofColor::magenta);
   mColors.emplace_back("Cyan", ofColor::cyan);
}

LabelDisplay::~LabelDisplay()
{
}

void LabelDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mShowControlsCheckbox, "showcontrols", &mShowControls);
   mShowControlsCheckbox->SetDisplayText(false);
   UIBLOCK_SHIFTRIGHT();
   TEXTENTRY(mLabelEntry, "label", 12, mLabel);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mLabelSizeSlider, "size", &mLabelSize, 25, 500);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mLabelFontDropdown, "font", &mLabelFontIndex, 90);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mLabelColorDropdown, "color", &mLabelColorIndex, 72);
   ENDUIBLOCK(mWidth, mHeight);

   for (int i = 0; i < mFonts.size(); i++)
      mLabelFontDropdown->AddLabel(mFonts[i].name, i);

   for (int i = 0; i < mColors.size(); i++)
      mLabelColorDropdown->AddLabel(mColors[i].name, i);
}

void LabelDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (mShowControls)
   {
      mShowControlsCheckbox->Draw();
      mLabelEntry->Draw();
      mLabelSizeSlider->Draw();
      mLabelFontDropdown->Draw();
      mLabelColorDropdown->Draw();
   }

   ofSetColor(mLabelColor);
   mLabelFont.DrawString(mLabel, mLabelSize, 2, (mLabelFont.GetStringHeight(mLabel, mLabelSize) / 1.25) + (mShowControls ? mHeight : 0));
}

void LabelDisplay::GetModuleDimensions(float& w, float& h)
{
   w = MAX((mShowControls ? mWidth : 0), mLabelFont.GetStringWidth(mLabel, mLabelSize) + 6);
   h = (mShowControls ? mHeight : 0) + (3 + mLabelFont.GetStringHeight(mLabel, mLabelSize)) * 1.05;
}

void LabelDisplay::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void LabelDisplay::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mLabelFontDropdown && mLabelFontIndex >= 0 && mLabelFontIndex < mFonts.size())
      mLabelFont = mFonts[mLabelFontIndex].font;
   else if (list == mLabelColorDropdown && mLabelColorIndex >= 0 && mLabelColorIndex < mColors.size())
      mLabelColor = mColors[mLabelColorIndex].color;
}

void LabelDisplay::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mShowControlsCheckbox)
   {
      mLabelEntry->SetNoHover(!mShowControls);
      mLabelSizeSlider->SetNoHover(!mShowControls);
      mLabelFontDropdown->SetNoHover(!mShowControls);
      mLabelColorDropdown->SetNoHover(!mShowControls);
   }
}

void LabelDisplay::TextEntryComplete(TextEntry* entry)
{
}
