//
//  TitleBar.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/30/14.
//
//

#ifndef __Bespoke__TitleBar__
#define __Bespoke__TitleBar__

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"

class ModuleFactory;
class TitleBar;
class HelpDisplay;

class SpawnList
{
public:
   SpawnList(TitleBar* owner, int x, int y, string label);
   void SetList(vector<string> spawnables, bool vstList);
   void OnSelection(DropdownList* list);
   void SetPosition(int x, int y);
   void SetPositionRelativeTo(SpawnList* list);
   void Draw();
private:
   IDrawableModule* Spawn();
   
   string mLabel;
   std::vector<string> mSpawnables;
   int mSpawnIndex;
   DropdownList* mSpawnList;
   TitleBar* mOwner;
   ofVec2f mPos;
   bool mVstList;
};

class TitleBar : public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener
{
public:
   TitleBar();
   ~TitleBar();
   
   string GetTitleLabel() override { return ""; }
   void CreateUIControls() override;
   bool HasTitleBar() const override { return false; }
   bool AlwaysOnTop() override { return true; }
   bool IsSingleton() const override { return true; }
   
   void SetModuleFactory(ModuleFactory* factory);
   void ListLayouts();
   
   bool IsSaveable() override { return false; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& x, int&y) override;
   
   bool HiddenByZoom() const;
   
   SpawnList mInstrumentModules;
   SpawnList mNoteModules;
   SpawnList mSynthModules;
   SpawnList mAudioModules;
   SpawnList mModulatorModules;
   SpawnList mOtherModules;
   SpawnList mVstPlugins;
   
   ClickButton* mSaveLayoutButton;
   ClickButton* mSaveStateButton;
   ClickButton* mLoadStateButton;
   ClickButton* mWriteAudioButton;
   ClickButton* mQuitButton;
   DropdownList* mLoadLayoutDropdown;
   ClickButton* mDisplayHelpButton;
   int mLoadLayoutIndex;
   
   HelpDisplay* mHelpDisplay;
};

extern TitleBar* TheTitleBar;

#endif /* defined(__Bespoke__TitleBar__) */


