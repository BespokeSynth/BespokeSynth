/*
  ==============================================================================

    Prefab.h
    Created: 25 Sep 2016 10:14:15am
    Author:  Ryan Challinor

  ==============================================================================
*/

#ifndef PREFAB_H_INCLUDED
#define PREFAB_H_INCLUDED

#include "IDrawableModule.h"
#include "ClickButton.h"

class PatchCableSource;

class Prefab : public IDrawableModule, public IButtonListener
{
public:
   Prefab();
   ~Prefab();
   static IDrawableModule* Create() { return new Prefab(); }
   
   string GetTitleLabel() override { return "prefab"; }
   void CreateUIControls() override;
   bool AlwaysOnBottom() override { return true; }
   void Move(float moveX, float moveY) override;
   
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(int& width, int& height) override;
   
   void SavePrefab(string savePath);
   void LoadPrefab(string loadPath);
   
   PatchCableSource* mModuleCable;
   list<IDrawableModule*> mModules;
   ClickButton* mSaveButton;
   ClickButton* mLoadButton;
};



#endif  // PREFAB_H_INCLUDED
