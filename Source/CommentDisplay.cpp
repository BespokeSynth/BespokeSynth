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
//  CommentDisplay.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/1/15.
//
//

#include "CommentDisplay.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

#include <cstring>

CommentDisplay::CommentDisplay()
{
}

CommentDisplay::~CommentDisplay()
{
}

void CommentDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mCommentEntry = new TextEntry(this, "comment", 2, 2, MAX_TEXTENTRY_LENGTH - 1, &mComment);
   mCommentEntry->SetFlexibleWidth(true);
}

void CommentDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mCommentEntry->Draw();
}

void CommentDisplay::GetModuleDimensions(float& w, float& h)
{
   mCommentEntry->GetDimensions(w, h);
   w += 4;
   h += 4;
}

void CommentDisplay::TextEntryComplete(TextEntry* entry)
{
}

void CommentDisplay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("comment", moduleInfo, "insert comment here");

   SetUpFromSaveData();
}

void CommentDisplay::SetUpFromSaveData()
{
   mComment = mModuleSaveData.GetString("comment");
}

void CommentDisplay::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["comment"] = mComment;
}
