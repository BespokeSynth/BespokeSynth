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
//  PanicButton.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/31/14.
//
//

#include "PanicButton.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

PanicButton::PanicButton()
{
}

PanicButton::~PanicButton()
{
}

void PanicButton::OnClicked(float x, float y, bool right)
{
   TheSynth->LoadLayout(ofToDataPath("daftpunk.json"));
}

void PanicButton::DrawModule()
{
   gFont.DrawString("If anything goes horribly awry, click this\nand the party will be restored", 13, 5, 12);

   ofPushStyle();
   ofFill();
   ofSetColor(70, 0, 0);
   ofRect(10, 40, 270, 80);
   ofSetColor(255, 0, 0);
   gFont.DrawString("PANIC", 106, 15, 115);
   ofPopStyle();
}