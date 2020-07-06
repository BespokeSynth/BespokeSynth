//
//  CommentDisplay.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/1/15.
//
//

#ifndef __Bespoke__CommentDisplay__
#define __Bespoke__CommentDisplay__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "TextEntry.h"

class CommentDisplay : public IDrawableModule, public ITextEntryListener
{
public:
   CommentDisplay();
   virtual ~CommentDisplay();
   static IDrawableModule* Create() { return new CommentDisplay(); }
   
   string GetTitleLabel() override { return "comment"; }
   void CreateUIControls() override;
   
   void TextEntryComplete(TextEntry* entry) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;
   
   char mComment[MAX_TEXTENTRY_LENGTH];
   TextEntry* mCommentEntry;
};



#endif /* defined(__Bespoke__CommentDisplay__) */
