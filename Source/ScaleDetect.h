//
//  ScaleDetect.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/10/13.
//
//

#ifndef __modularSynth__ScaleDetect__
#define __modularSynth__ScaleDetect__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Scale.h"
#include "DropdownList.h"

struct MatchingScale
{
   MatchingScale(int root, string type) : mRoot(root), mType(type) {}
   int mRoot;
   string mType;
};

class ScaleDetect : public NoteEffectBase, public IDrawableModule, public IButtonListener, public IDropdownListener
{
public:
   ScaleDetect();
   static IDrawableModule* Create() { return new ScaleDetect(); }
   
   string GetTitleLabel() override { return "detect"; }
   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   bool ScaleSatisfied(int root, string type);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 140; height = 36; }
   bool Enabled() const override { return true; }

   vector<int> mNotes;
   ClickButton* mResetButton;
   int mLastNote;
   bool mDoDetect;
   
   ofMutex mListMutex;
   DropdownList* mMatchesDropdown;
   int mSelectedMatch;
};

#endif /* defined(__modularSynth__ScaleDetect__) */

