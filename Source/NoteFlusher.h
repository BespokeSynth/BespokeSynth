//
//  NoteFlusher.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/23/14.
//
//

#ifndef __Bespoke__NoteFlusher__
#define __Bespoke__NoteFlusher__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ClickButton.h"

class NoteFlusher : public NoteEffectBase, public IDrawableModule, public IButtonListener
{
public:
   NoteFlusher();
   static IDrawableModule* Create() { return new NoteFlusher(); }
   
   string GetTitleLabel() override { return "flusher"; }
   void CreateUIControls() override;
   
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 18; }
   bool Enabled() const override { return mEnabled; }
   
   ClickButton* mFlushButton;
};


#endif /* defined(__Bespoke__NoteFlusher__) */
