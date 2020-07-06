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

CommentDisplay::CommentDisplay()
: mCommentEntry(nullptr)
{
   mComment[0] = 0;
}

CommentDisplay::~CommentDisplay()
{
}

void CommentDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mCommentEntry = new TextEntry(this,"comment",2,2,MAX_TEXTENTRY_LENGTH-1,mComment);
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
   strcpy(mComment, mModuleSaveData.GetString("comment").c_str());
}

void CommentDisplay::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["comment"] = mComment;
}
