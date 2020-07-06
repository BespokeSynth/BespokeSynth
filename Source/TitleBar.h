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
struct SpawnListManager;

class SpawnList
{
public:
   SpawnList(IDropdownListener* owner, SpawnListManager* listManager, int x, int y, string label);
   void SetList(vector<string> spawnables, string overrideModuleType);
   void OnSelection(DropdownList* list);
   void SetPosition(int x, int y);
   void SetPositionRelativeTo(SpawnList* list);
   void Draw();
   DropdownList* GetList() { return mSpawnList; }
   IDrawableModule* Spawn();
   
private:
   string mLabel;
   std::vector<string> mSpawnables;
   int mSpawnIndex;
   DropdownList* mSpawnList;
   IDropdownListener* mOwner;
   SpawnListManager* mListManager;
   ofVec2f mPos;
   string mOverrideModuleType;
};

struct SpawnListManager
{
   SpawnListManager(IDropdownListener* owner);
   
   void SetModuleFactory(ModuleFactory* factory);
   void SetUpVstDropdown(bool rescan);
   vector<SpawnList*> GetDropdowns() { return mDropdowns; }
   
   SpawnList mInstrumentModules;
   SpawnList mNoteModules;
   SpawnList mSynthModules;
   SpawnList mAudioModules;
   SpawnList mModulatorModules;
   SpawnList mOtherModules;
   SpawnList mVstPlugins;
   SpawnList mPrefabs;
   
private:
   vector<SpawnList*> mDropdowns;
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
   
   void SetModuleFactory(ModuleFactory* factory) { mSpawnLists.SetModuleFactory(factory); }
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
   void GetModuleDimensions(float& width, float& height) override;
   
   bool HiddenByZoom() const;
   
   ClickButton* mSaveLayoutButton;
   ClickButton* mResetLayoutButton;
   ClickButton* mSaveStateButton;
   ClickButton* mLoadStateButton;
   ClickButton* mWriteAudioButton;
   ClickButton* mQuitButton;
   DropdownList* mLoadLayoutDropdown;
   ClickButton* mDisplayHelpButton;
   Checkbox* mEventLookaheadCheckbox;
   int mLoadLayoutIndex;
   
   HelpDisplay* mHelpDisplay;
   
   SpawnListManager mSpawnLists;
};

extern TitleBar* TheTitleBar;

#endif /* defined(__Bespoke__TitleBar__) */


